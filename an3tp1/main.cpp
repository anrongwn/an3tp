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

#include "hiredis.h"

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

std::atomic_bool g_exit_flag(false);

static void signal_handler(int signum) {
    std::cout << "recv " << signum << " signal" << std::endl;
    switch (signum) {
    case SIGINT:
        g_exit_flag.store(true);
        // vrl_stop_notify(vrl, 1);
        // vrl_stop_event_loop(vrl, 0);
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

int main(int argc, char *argv[]) {
#ifdef DEBUG
    std::cout << "anredistp debug mode." << std::endl;
#endif

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

    

    auto start = std::chrono::system_clock::now();

    /*
        std::string out("{\"result\":\"1\"}");
        cJSON *json = cJSON_Parse(out.c_str());
        std::cout << "json = " << cJSON_PrintUnformatted(json) << std::endl;
        cJSON_Delete(json);
    */
    // std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60));
   
    unsigned int j, isunix = 0;
    redisContext * c = nullptr;
    redisReply *reply = nullptr;
    
    const char *hostname = (argc > 1) ? argv[1] : "192.168.8.132";

    if (argc > 2) {
        if (*argv[2] == 'u' || *argv[2] == 'U') {
            isunix = 1;
            /* in this case, host is the path to the unix socket */
            printf("Will connect to unix socket @%s\n", hostname);
        }
    }

    int port = (argc > 2) ? atoi(argv[2]) : 6379;

    struct timeval timeout = { 1, 500000 }; // 1.5 seconds
    if (isunix) {
        c = redisConnectUnixWithTimeout(hostname, timeout);
    } else {
        c = redisConnectWithTimeout(hostname, port, timeout);
    }
    if (c == NULL || c->err) {
        if (c) {
            printf("Connection error: %s\n", c->errstr);
            redisFree(c);
        } else {
            printf("Connection error: can't allocate redis context\n");
        }
        exit(-1);
    }

    //login
    reply = (redisReply*)redisCommand(c, "AUTH 924(@$");
    switch (reply->type)
    {
    case REDIS_REPLY_ERROR :
        /* code */
        printf("AUTH : error(%d) %s\n", REDIS_REPLY_ERROR, reply->str);
        freeReplyObject(reply);
        exit(-1);
        break;
    
    default:
        printf("AUTH: %s\n", reply->str);
        freeReplyObject(reply);
        break;
    }

    //select 5 db
    reply = (redisReply*)redisCommand(c, "SELECT 5");
    switch (reply->type)
    {
    case REDIS_REPLY_ERROR :
        /* code */
        printf("SELECT 5 : error(%d) %s\n", REDIS_REPLY_ERROR, reply->str);
        freeReplyObject(reply);
        exit(-1);
        break;
    
    default:
        printf("SELECT 5: %s\n", reply->str);
        freeReplyObject(reply);
        break;
    }

    /* PING server */
    reply = (redisReply*)redisCommand(c,"PING");
    printf("PING: %d, %s\n",reply->type, reply->str);
    freeReplyObject(reply);

    /* Set a key */
    reply = (redisReply*)redisCommand(c,"SET %s %s", "foo", "hello world");
    printf("SET: %d, %s\n", reply->type, reply->str);
    freeReplyObject(reply);

    /* Set a key using binary safe API */
    reply = (redisReply*)redisCommand(c,"SET %b %b", "bar", (size_t) 3, "hello", (size_t) 5);
    printf("SET (binary API): %d, %s\n", reply->type, reply->str);
    freeReplyObject(reply);

    /* Try a GET and two INCR */
    reply = (redisReply*)redisCommand(c,"GET foo");
    printf("GET foo: %d, %s\n", reply->type, reply->str);
    freeReplyObject(reply);

    reply = (redisReply*)redisCommand(c,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);
    /* again ... */
    reply = (redisReply*)redisCommand(c,"INCR counter");
    printf("INCR counter: %lld\n", reply->integer);
    freeReplyObject(reply);

    /* Create a list of numbers, from 0 to 9 */
    reply = (redisReply*)redisCommand(c,"DEL mylist");
    freeReplyObject(reply);
    for (j = 0; j < 10; j++) {
        char buf[64];

        snprintf(buf,64,"%u",j);
        reply = (redisReply*)redisCommand(c,"LPUSH mylist element-%s", buf);
        freeReplyObject(reply);
    }

    /* Let's check what we have inside the list */
    reply = (redisReply*)redisCommand(c,"LRANGE mylist 0 -1");
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (j = 0; j < reply->elements; j++) {
            printf("%u) %s\n", j, reply->element[j]->str);
        }
    }
    freeReplyObject(reply);

    /* Disconnects and frees the context */
    redisFree(c);

    std::cout << "anredistp exit." << std::endl;
    return 0;
}