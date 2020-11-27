#include "an_nng.h"
#include <atomic>

extern volatile std::atomic_bool g_exit_flag;

static void fatal(const char *func, int rv) {
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

static void showdate(time_t now) {
	struct tm *info = localtime(&now);
	printf("%s", asctime(info));
}

int server(const char *url) {
	nng_socket sock;
	int rv;

	if ((rv = nng_rep0_open(&sock)) != 0) {
		fatal("nng_rep0_open", rv);
	}
	if ((rv = nng_listen(sock, url, NULL, 0)) != 0) {
		fatal("nng_listen", rv);
	}

	// for (;;) {
	while (!g_exit_flag) {
		char *buf = NULL;
		size_t sz = 0;
		uint64_t val = 0;
		if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
			fatal("nng_recv", rv);
		}
		if ((sz == sizeof(uint64_t)) && ((GET64(buf, val)) == DATECMD)) {
			time_t now = 0;
			printf("SERVER: RECEIVED DATE REQUEST\n");
			now = time(&now);
			printf("SERVER: SENDING DATE: ");
			showdate(now);

			// Reuse the buffer.  We know it is big enough.
			PUT64(buf, (uint64_t)now);
			rv = nng_send(sock, buf, sz, NNG_FLAG_ALLOC);
			if (rv != 0) {
				fatal("nng_send", rv);
			}
			continue;
		}

		// Unrecognized command, so toss the buffer.
		nng_free(buf, sz);
	}

	return 0;
}

int client(const char *url) {
	nng_socket sock;
	int rv;
	size_t sz;
	char *buf = NULL;
	uint8_t cmd[sizeof(uint64_t)];

	PUT64(cmd, DATECMD);

	if ((rv = nng_req0_open(&sock)) != 0) {
		fatal("nng_socket", rv);
	}
	if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
		fatal("nng_dial", rv);
	}

	while (!g_exit_flag) {
		printf("CLIENT: SENDING DATE REQUEST\n");
		if ((rv = nng_send(sock, cmd, sizeof(cmd), 0)) != 0) {
			fatal("nng_send", rv);
		}
		if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
			fatal("nng_recv", rv);
		}

		if (sz == sizeof(uint64_t)) {
			uint64_t now;
			GET64(buf, now);
			printf("CLIENT: RECEIVED DATE: ");
			showdate((time_t)now);
		} else {
			printf("CLIENT: GOT WRONG SIZE!\n");
		}

		nng_msleep(10);
	}
	// This assumes that buf is ASCIIZ (zero terminated).
	nng_free(buf, sz);
	nng_close(sock);
	return (0);
}
