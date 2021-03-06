#include <errno.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include <c_procs/netutils/netutils.h>
#include <c_procs/strutils.h>

int set_reuseaddr(int sock_fd) {
	int reuse = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
	    0)
		return -1;
	return 0;
}

#ifdef SO_REUSEPORT
/* since Linux 3.9 */
int set_reuseport(int sock_fd) {
	int reuse = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) <
	    0)
		return -1;
	return 0;
}
#endif

int set_recv_timeout(int sock_fd, struct timeval *tv) {
	return setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, tv,
	                  sizeof(struct timeval));
}

int set_recv_timeout_microsec(int sock_fd, size_t microseconds) {
	struct timeval tv;
	tv.tv_sec = (ssize_t) microseconds / 1000000;
	tv.tv_usec = (ssize_t) microseconds % 1000000;
	return setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv,
	                  sizeof(struct timeval));
}

int set_send_timeout(int sock_fd, struct timeval *tv) {
	return setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, tv,
	                  sizeof(struct timeval));
}

int set_send_timeout_microsec(int sock_fd, size_t microseconds) {
	struct timeval tv;
	tv.tv_sec = (ssize_t) microseconds / 1000000;
	tv.tv_usec = (ssize_t) microseconds % 1000000;
	return setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, &tv,
	                  sizeof(struct timeval));
}


int set_keepalive(int sock_fd) {
	int enable = 1;
	return setsockopt(sock_fd, SOL_SOCKET, SO_KEEPALIVE, &enable, sizeof(int));
}

int set_keepalive_idle(int sock_fd, int idle) {
	return setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int));
}

int set_keepalive_interval(int sock_fd, int interval) {
	return setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPINTVL, &interval,
	                  sizeof(int));
}

int set_keepalive_probes(int sock_fd, int probes) {
	return setsockopt(sock_fd, IPPROTO_TCP, TCP_KEEPCNT, &probes, sizeof(int));
}

ssize_t recv_try(int sock_fd, char *buf, size_t max_len, int flag,
                 size_t *rsize, short try_count, const char end) {
	ssize_t n = 0;
	char *  pbuf = buf;
	*rsize = 0;

	for (short i = 0; i < try_count; i++) {
		n = recv(sock_fd, pbuf, max_len, flag);
		if (n < 0) {
			return n;
		} else {
			pbuf += n;
			*rsize += (size_t) n;
			max_len -= (size_t) n;
			if (max_len == 0 || buf[*rsize - 1] == end) {
				return (ssize_t) *rsize;
			}
		}
	}
	return (ssize_t) *rsize;
}

ssize_t send_try(int sock_fd, char *buf, size_t len, int flag, size_t *wsize,
                 short try_count) {
	ssize_t n = 0;
	char *  pbuf = buf;
	*wsize = 0;

	for (short i = 0; i < try_count; i++) {
		n = send(sock_fd, pbuf, len, flag);
		if (n <= 0) {
			return n;
		} else {
			pbuf += n;
			*wsize += (size_t) n;
			len -= (size_t) n;
			if (len == 0) {
				return (ssize_t) *wsize;
			}
		}
	}
	return (ssize_t) *wsize;
}

int validate_ipv4(char *str) {
	int  dots = 0, digits = 0;
	char dbuf[4];

	if (str == NULL)
		return 0;

	while (1) {
		/* after parsing string, it must contain only digits */
		if (*str == '\0' || *str == '.') {
			long  n;
			char *tmp;
			if (digits == 0 || dots > 4)
				return 0;
			else if (*str == '\0' && (dots == 0 || dots == 3))
				return 1;
			else if (*str == '.' && dots == 3)
				return 0;
			dbuf[digits] = '\0';
			digits = 0;
			n = str2l(dbuf, &tmp, 10);
			if (errno != 0)
				return 0;
			if ((dots == 0 || dots == 3) && (n <= 0 || n >= 254))
				return 0;
			else if (n < 0 || n > 254)
				return 0;
			dots++;
		} else if (*str >= '0' && *str <= '9' && dots < 4)
			dbuf[digits++] = *str;
		else
			return 0;
		str++;
	}
}
