#include <time.h>

#include "stats.h"

void stats_report(TcpConnection *tcp_conn_list)
{
    LOG("Stats | Sessions: %d. TCP connections: %d.\n",
        tnfs_session_count(),
        tcp_connections_count(tcp_conn_list));
}

uint8_t tcp_connections_count(TcpConnection *tcp_conn_list)
{
	uint16_t count = 0;
	for (uint16_t i = 0; i < MAX_TCP_CONN; i++)
	{
		if (tcp_conn_list[i].cli_fd)
		{
			count++;
		}
	}
	return count;
}