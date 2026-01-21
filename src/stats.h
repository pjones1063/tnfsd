#ifndef _STATS_H
#define _STATS_H

#include "config.h"
#include "datagram.h"
#include "log.h"
#include "session.h"
#include "tnfs.h"

void stats_report(TcpConnection *tcp_conn_list);
uint8_t tcp_connections_count(TcpConnection *tcp_conn_list);

#endif
