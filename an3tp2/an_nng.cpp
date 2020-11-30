#include "an_nng.h"
#include <algorithm>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include "CJsonObject.hpp"

#include <sys/time.h>
#include "an_fmtput.h"

#pragma GCC diagnostic ignored "-Wunused"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

extern volatile std::atomic_bool g_exit_flag;

//cmd object
using fn_cmd_cb = std::function<std::string (const std::string &, const std::string &id) >;

//cmd name -- cmd object map
using cmd_map = std::map<std::string, fn_cmd_cb>;
static cmd_map g_cmd;

struct get_data_cmd{
	std::string operator()(const std::string &arg,  const std::string &id){

		g_console->warn("==== SERVER: RECEIVED {} get_data {} REQUEST...", id, arg);
		
		struct timeval tv ={0x00};
    	gettimeofday(&tv, nullptr);
		struct tm * time_ptr = localtime(&tv.tv_sec);

		std::string echo = fmt::format("{}-{}-{} {}:{}:{}.{}", time_ptr->tm_year + 1900,
            time_ptr->tm_mon + 1,
            time_ptr->tm_mday,
            time_ptr->tm_hour,
            time_ptr->tm_min,
            time_ptr->tm_sec,
			tv.tv_usec);



		return echo;
		
	}
};

static void fatal(const char *func, int rv) {
	fprintf(stderr, "%s: %s\n", func, nng_strerror(rv));
	exit(1);
}

static void showdate(time_t now) {
	struct tm *info = localtime(&now);
	printf("%s", asctime(info));
}

int server(const char *url, const char *name) {
	nng_socket sock;
	int rv;

	get_data_cmd cmd1;
	g_cmd["get_data"] = cmd1;
	

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
		
		//g_console->info("nng_recv {}", std::string(buf, sz));
		
		neb::CJsonObject json_cmd(std::string(buf, sz));
		nng_free(buf, sz);
		if (json_cmd.IsEmpty()){
			fatal("nng_recv json-rpc cmd is null.", rv);
		}

		std::string method ;
		json_cmd.Get("method", method);
		std::string params;
		json_cmd.Get("params", params);
		std::string id;
		json_cmd.Get("id", id);
		std::string echo = g_cmd[method](params, id);
		
		neb::CJsonObject json_result;
		json_result.Add("jsonrpc", "2.0");
		json_result.Add("result", echo);
		json_result.Add("id", id);
		std::string result = json_result.ToString();

		//rv = nng_send(sock, (void*)buf, sizof(buf), NNG_FLAG_ALLOC);
		rv = nng_send(sock, (void*)result.data(), result.length(), 0);
		if (rv != 0) {
			fatal("nng_send", rv);
		}
		g_console->info("===={} SERVER SENDING DATE: {}", name, result);
		/*
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
		*/

		// Unrecognized command, so toss the buffer.
		//nng_free(buf, sz);
	}

	return 0;
}

int client(const char *url, const char * name) {
	static volatile unsigned long g_cmd_id_ = 0;
	
	nng_socket sock;
	int rv;
	size_t sz;
	char *buf = NULL;

	// uint8_t cmd[sizeof(uint64_t)];
	// PUT64(cmd, DATECMD);

	if ((rv = nng_req0_open(&sock)) != 0) {
		fatal("nng_socket", rv);
	}
	if ((rv = nng_dial(sock, url, NULL, 0)) != 0) {
		fatal("nng_dial", rv);
	}

	
	
	while (!g_exit_flag) {
		neb::CJsonObject rpc_cmd;
		rpc_cmd.Add("jsonrpc", "2.0");
		rpc_cmd.Add("method", "get_data");
		rpc_cmd.AddEmptySubArray("params");
		//rpc_cmd["params"].Add(name);
		
		std::string req_id = fmt::format("{}-{}", name, ++g_cmd_id_);
		rpc_cmd.Add("id", req_id);
		
		std::string json = rpc_cmd.ToString();
		g_console->warn("{} CLIENT: SENDING get_data REQUEST {}", name, json);
		
		//printf("CLIENT: SENDING DATE REQUEST\n");
		
		if ((rv = nng_send(sock, (void *)json.data(),json.length(), 0)) != 0) {
			fatal("nng_send", rv);
		}
		// if ((rv = nng_send(sock, cmd, sizeof(cmd), 0)) != 0) {
		// 	fatal("nng_send", rv);
		// }
		
		if ((rv = nng_recv(sock, &buf, &sz, NNG_FLAG_ALLOC)) != 0) {
			fatal("nng_recv", rv);
		}

		g_console->info("{} CLIENT RECEIVED DATE: {}", name, std::string(buf, sz));
		
		/*
		if (sz == sizeof(uint64_t)) {
			uint64_t now;
			GET64(buf, now);
			printf("CLIENT: RECEIVED DATE: ");
			showdate((time_t)now);
		} else {
			printf("CLIENT: GOT WRONG SIZE!\n");
		}
		*/

		nng_msleep(10);
	}
	// This assumes that buf is ASCIIZ (zero terminated).
	nng_free(buf, sz);
	nng_close(sock);
	return (0);
}
