//#include "test1.h"
//#include "test2.h"
#include "cmdline/cmdline.h"
#include <atomic>
#include <cstdint>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

//#include "anlog.h"
#include "cJSON.h"
#include "gperftools/malloc_extension.h"
#include "uv.h"
#include <chrono>
#include <cstring>
#include <mutex>
#include <random>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>

//#include "hiredis.h"
#include <pthread.h>
#include <unistd.h>

#include "an_fmtput.h"
#include "an_nng.h"

/*
typedef struct vrl_s {
	uv_loop_t *loop;
} vrl_t;
*/
#pragma GCC diagnostic ignored "-Wunused"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"

typedef struct test_s {
	int id;
} test_t;

volatile std::atomic_bool g_exit_flag(false);

static void signal_handler(int signum) {
	std::cout << "recv " << signum << " signal" << std::endl;
	switch (signum) {
	case SIGINT:
		g_exit_flag.store(true);
		// vrl_stop_notify(vrl, 1);
		// vrl_stop_event_loop(vrl, 0);

		nng_closeall();
		break;
	case SIGPIPE:
		// g_exit_flag.store(true);
		// abort();
		break;

	default:
		break;
	}
}

inline static long get_id() {
	auto value =
		std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()).time_since_epoch();
	// long duration = value.count();

	return value.count();
}

// test task
static std::mutex g_mutex_;

static int task(int mode, const char *url, const char *params) {
	static int i = 0;

	return 0;
}

static void thread_cb(int mode, const char *url, const char *params) {

	std::default_random_engine random(time(NULL));
	std::uniform_int_distribution<int> dis1(30, 180);

	while (!g_exit_flag.operator bool()) {
		task(mode, url, params);

		std::this_thread::sleep_for(std::chrono::milliseconds(dis1(random)));
	}
}

// main
int main(int argc, char *argv[]) {
	g_console->set_pattern("[%^%l%$] %v");

#ifdef DEBUG
	// std::cout << "an3tp2 debug mode." << std::endl;
	g_console->info("an3tp2 debug mode.");
#endif
	auto start = std::chrono::system_clock::now();
	signal(SIGINT, signal_handler);
	// signal(SIGPIPE, signal_handler);

#ifndef DEBUG
	//解析命令行输入参数
	cmdline::parser a;
	//解析输入参数
	a.add<std::string>("d", 'd', "目的URL / distination URL", true, "http://www.badui.com");
	a.add<int>("n", 'n', "访问次数 / visit times", false, 1, cmdline::range(1, INT32_MAX));
	a.add<int>("m", 'm', "访问模式 / visit mode(0 sync, 1 async)", false, 0, cmdline::range(0, 1));
	a.add<int>("t", 't', "间隔时间ms / interval time ms", false, 40, cmdline::range(0, INT32_MAX));
	a.parse_check(argc, argv);

	////获取输入参数
	std::string strUrl = a.get<std::string>("d");
	int times = a.get<int>("n");
	int mode = a.get<int>("m");
	int interval = a.get<int>("t");

	std::cout << "URL=" << strUrl << ",times=" << times << ",mode=" << mode << ",interval=" << interval << std::endl;
#else
	std::string strUrl = "https://192.168.75.30:8443/api/dep_setting_stub/l/";
	int times = 20000;
	int mode = 1;
	int interval = 40;
#endif

	if ((argc >= 4) && (strcmp(CLIENT, argv[1]) == 0)) {

		return (client(argv[2], argv[3]));
	}

	if ((argc >= 4) && (strcmp(SERVER, argv[1]) == 0)){
		
		return (server(argv[2], argv[3]));
	}
	
	fprintf(stderr, "Usage: reqrep %s|%s <URL> %s...\n", CLIENT, SERVER, "name");

	/*while (!g_exit_flag) {

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}*/
	std::cout << "stop." << std::endl;

	nng_fini();
	return 0;

	/*
	std::string out("{\"result\":\"1\"}");
	cJSON *json = cJSON_Parse(out.c_str());
	std::cout << "json = " << cJSON_PrintUnformatted(json) << std::endl;
	cJSON_Delete(json);
	*/

	// std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60));

	std::cout << "an3tp2 exit." << std::endl;
	return 0;
}