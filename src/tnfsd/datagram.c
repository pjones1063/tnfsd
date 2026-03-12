/* The MIT License
Copyright (c) 2010 Dylan Smith
... (License text remains unchanged) ...
*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#ifdef UNIX
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif
#endif

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include "tnfs.h"
#include "datagram.h"
#include "log.h"
#include "endian.h"
#include "session.h"
#include "errortable.h"
#include "directory.h"
#include "tnfs_file.h"

int sockfd;		 /* UDP global socket file descriptor */
int tcplistenfd; /* TCP listening socket file descriptor */
volatile int g_tnfs_running = 0;

tnfs_cmdfunc dircmd[NUM_DIRCMDS] =
    {&tnfs_opendir, &tnfs_readdir, &tnfs_closedir,
     &tnfs_mkdir, &tnfs_rmdir, &tnfs_telldir, &tnfs_seekdir,
     &tnfs_opendirx, &tnfs_readdirx};

tnfs_cmdfunc filecmd[NUM_FILECMDS] =
    {&tnfs_open_deprecated, &tnfs_read, &tnfs_write, &tnfs_close,
     &tnfs_stat, &tnfs_lseek, &tnfs_unlink, &tnfs_chmod, &tnfs_rename,
     &tnfs_open};

const char *sesscmd_names[NUM_SESSCMDS] = {"TNFS_MOUNT", "TNFS_UMOUNT"};
const char *dircmd_names[NUM_DIRCMDS] = {"TNFS_OPENDIR", "TNFS_READDIR", "TNFS_CLOSEDIR", "TNFS_MKDIR", "TNFS_RMDIR", "TNFS_TELLDIR", "TNFS_SEEKDIR", "TNFS_OPENDIRX", "TNFS_READDIRX"};
const char *filecmd_names[NUM_FILECMDS] = {"TNFS_OPENFILE_OLD", "TNFS_READ", "TNFS_WRITE", "TNFS_CLOSE", "TNFS_STAT", "TNFS_SEEK", "TNFS_UNLINK", "TNFS_CHMOD", "TNFS_RENAME", "TNFS_OPEN"};

const char *get_cmd_name(uint8_t cmd) {
    uint8_t class = cmd & 0xF0;
    uint8_t index = cmd & 0x0F;
    if(class == CLASS_FILE && index < NUM_FILECMDS) return filecmd_names[index];
    if(class == CLASS_DIRECTORY && index < NUM_DIRCMDS) return dircmd_names[index];
    if(class == CLASS_SESSION && index < NUM_SESSCMDS) return sesscmd_names[index];
    return "UNKNOWN_CMD";
}

int tnfs_sockinit(int port) {
    struct sockaddr_in servaddr;
#ifdef WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) return -1;
#endif
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) return -1;

    tcplistenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcplistenfd < 0) return -1;
    int reuseaddr = 1;
    setsockopt(tcplistenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuseaddr, sizeof(reuseaddr));
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    if (bind(tcplistenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) return -1;
    listen(tcplistenfd, 5);
    return 0;
}

void tnfs_mainloop() {
    int readyfds, i;
    fd_set fdset, errfdset;
    TcpConnection tcpsocks[MAX_TCP_CONN];
    struct timeval select_timeout;
    time_t last_stats_report = 0, now = 0;
    memset(&tcpsocks, 0, sizeof(tcpsocks));
    g_tnfs_running = 1;

    while (g_tnfs_running) {
        FD_ZERO(&fdset);
        FD_SET(sockfd, &fdset);
        FD_SET(tcplistenfd, &fdset);
        for (i = 0; i < MAX_TCP_CONN; i++) {
            if (tcpsocks[i].cli_fd) FD_SET(tcpsocks[i].cli_fd, &fdset);
        }
        select_timeout.tv_sec = 1;
        readyfds = select(FD_SETSIZE, &fdset, NULL, NULL, &select_timeout);
        if (readyfds == SOCKET_ERROR) break;
        if (readyfds == 0) continue;
        if (FD_ISSET(sockfd, &fdset)) tnfs_handle_udpmsg();
        if (FD_ISSET(tcplistenfd, &fdset)) tcp_accept(&tcpsocks[0]);
        for (i = 0; i < MAX_TCP_CONN; i++) {
            if (tcpsocks[i].cli_fd && FD_ISSET(tcpsocks[i].cli_fd, &fdset)) tnfs_handle_tcpmsg(&tcpsocks[i]);
        }
        time(&now);
        if (STATS_INTERVAL > 0 && now - last_stats_report > STATS_INTERVAL) {
            stats_report(tcpsocks);
            last_stats_report = now;
        }
    }
}

void tcp_accept(TcpConnection *tcp_conn_list) {
    int acc_fd, i;
    struct sockaddr_in cliaddr;
#ifdef WIN32
    int cli_len = sizeof(cliaddr);
#else
    socklen_t cli_len = sizeof(cliaddr);
#endif
    acc_fd = accept(tcplistenfd, (struct sockaddr *)&cliaddr, &cli_len);
    if (acc_fd < 1) return;
    for (i = 0; i < MAX_TCP_CONN; i++) {
        if (tcp_conn_list[i].cli_fd == 0) {
            tcp_conn_list[i].cli_fd = acc_fd;
            tcp_conn_list[i].cliaddr = cliaddr;
            return;
        }
    }
    close(acc_fd);
}

void tnfs_handle_udpmsg() {
#ifdef WIN32
    int len;
#else
    socklen_t len;
#endif
    int rxbytes;
    struct sockaddr_in cliaddr;
    unsigned char rxbuf[MAXMSGSZ];
    len = sizeof(cliaddr);
    /* CRITICAL FIX: Do not subtract 1 from buffer size and do not manually null-terminate */
    rxbytes = recvfrom(sockfd, (char *)rxbuf, sizeof(rxbuf), 0, (struct sockaddr *)&cliaddr, &len);
    if (rxbytes >= TNFS_HEADERSZ) {
        tnfs_decode(&cliaddr, 0, rxbytes, rxbuf);
    }
}

void tnfs_handle_tcpmsg(TcpConnection *tcp_conn) {
    unsigned char buf[MAXMSGSZ];
    int sz;
    /* CRITICAL FIX: Do not manually null-terminate binary network buffers */
    sz = recv(tcp_conn->cli_fd, (char *)buf, sizeof(buf), 0);
    if (sz <= 0) {
        tnfs_reset_cli_fd_in_sessions(tcp_conn->cli_fd);
#ifdef WIN32
        closesocket(tcp_conn->cli_fd);
#else
        close(tcp_conn->cli_fd);
#endif
        tcp_conn->cli_fd = 0;
        return;
    }
    tnfs_decode(&tcp_conn->cliaddr, tcp_conn->cli_fd, sz, buf);
}

void tnfs_decode(struct sockaddr_in *cliaddr, int cli_fd, int rxbytes, unsigned char *rxbuf) {
    Header hdr;
    Session *sess;
    int sindex, cmdclass, cmdidx;
    int datasz = rxbytes - TNFS_HEADERSZ;
    unsigned char *databuf = rxbuf + TNFS_HEADERSZ;

    hdr.sid = tnfs16uint(rxbuf);
    hdr.seqno = *(rxbuf + 2);
    hdr.cmd = *(rxbuf + 3);
    hdr.ipaddr = cliaddr->sin_addr.s_addr;
    hdr.port = ntohs(cliaddr->sin_port);
    hdr.cli_fd = cli_fd;

    if (hdr.cmd != TNFS_MOUNT) {
        sess = tnfs_findsession_sid(hdr.sid, &sindex);
        if (sess == NULL) { tnfs_invalidsession(&hdr); return; }
        if (sess->ipaddr != hdr.ipaddr) return;
        sess->last_contact = time(NULL);
        sess->cli_fd = cli_fd;
    } else {
        tnfs_mount(&hdr, databuf, datasz);
        return;
    }

    if (hdr.seqno == sess->lastseqno) {
        tnfs_resend(sess, cliaddr, cli_fd);
        return;
    }

    cmdclass = hdr.cmd & 0xF0;
    cmdidx = hdr.cmd & 0x0F;
    switch (cmdclass) {
    case CLASS_SESSION:
        if (cmdidx == TNFS_UMOUNT) tnfs_umount(&hdr, sess, sindex);
        else tnfs_badcommand(&hdr, sess);
        break;
    case CLASS_DIRECTORY:
        if (cmdidx < NUM_DIRCMDS) (*dircmd[cmdidx])(&hdr, sess, databuf, datasz);
        break;
    case CLASS_FILE:
        if (cmdidx < NUM_FILECMDS) (*filecmd[cmdidx])(&hdr, sess, databuf, datasz);
        break;
    default:
        tnfs_badcommand(&hdr, sess);
    }
}

void tnfs_invalidsession(Header *hdr) {
    hdr->status = TNFS_EBADSESSION;
    tnfs_send(NULL, hdr, NULL, 0);
}

void tnfs_badcommand(Header *hdr, Session *sess) {
    hdr->status = TNFS_ENOSYS;
    tnfs_send(sess, hdr, NULL, 0);
}

void tnfs_send(Session *sess, Header *hdr, unsigned char *msg, int msgsz) {
    struct sockaddr_in cliaddr;
    unsigned char txbuf_nosess[5];
    unsigned char *txbuf = sess ? sess->lastmsg : txbuf_nosess;

    cliaddr.sin_family = AF_INET;
    cliaddr.sin_addr.s_addr = hdr->ipaddr;
    cliaddr.sin_port = htons(hdr->port);

    uint16tnfs(txbuf, sess ? hdr->sid : 0);
    *(txbuf + 2) = hdr->seqno;
    *(txbuf + 3) = hdr->cmd;
    *(txbuf + 4) = hdr->status;
    if (msg) memcpy(txbuf + 5, msg, msgsz);

    if (sess) {
        sess->lastmsgsz = TNFS_HEADERSZ + 1 + msgsz;
        sess->lastseqno = hdr->seqno;
    }

    if (hdr->cli_fd == 0) sendto(sockfd, WIN32_CHAR_P txbuf, msgsz + TNFS_HEADERSZ + 1, 0, (struct sockaddr *)&cliaddr, sizeof(cliaddr));
    else send(hdr->cli_fd, WIN32_CHAR_P txbuf, msgsz + TNFS_HEADERSZ + 1, 0);
}

void tnfs_resend(Session *sess, struct sockaddr_in *cliaddr, int cli_fd) {
    if (cli_fd == 0) sendto(sockfd, WIN32_CHAR_P sess->lastmsg, sess->lastmsgsz, 0, (struct sockaddr *)cliaddr, sizeof(struct sockaddr_in));
    else send(cli_fd, WIN32_CHAR_P sess->lastmsg, sess->lastmsgsz, 0);
}

void tnfs_shutdown_sockets() {
    if (sockfd > 0) {
#ifdef WIN32
        closesocket(sockfd);
#else
        close(sockfd);
#endif
        sockfd = 0;
    }
    if (tcplistenfd > 0) {
#ifdef WIN32
        closesocket(tcplistenfd);
#else
        close(tcplistenfd);
#endif
        tcplistenfd = 0;
    }
}
