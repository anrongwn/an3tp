#pragma once
#include <cstring>
#include <mutex>
#include <pthread.h>
#include <random>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <time.h>
#include <unistd.h>

#include "inproc.h"
#include "nn.h"
#include "pipeline.h"

#include "sqlite3.h"

/*
  此程序为nanomsg多线程一对一单向通信demo。
*/

//发送数据的socket初始化
int send_sock_init(int *sock);

//接收数据的socket初始化
int recieve_sock_init(int *sock);

//
class an_data_producer {
  public:
    an_data_producer();
    ~an_data_producer();

    sqlite3 *open_db(const char *dbname);

    int start(const char *dbname);
    int stop() {
        exit_ = 1;
        return exit_;
    }

  private:
    static int data_query_cb(void *context, int colcount, char **colvalues, char **colnames);

    static void *produce(void *args);

  private:
    sqlite3 *db_;
    pthread_t tid_;
    volatile int exit_;
};

class an_data_consumer {
  public:
    an_data_consumer();
    ~an_data_consumer();

    sqlite3 *open_db(const char *dbname);

    int start(const char *dbname);
    int stop() {
        exit_ = 1;
        return exit_;
    }

  private:
    static void *consume(void *args);

  private:
    sqlite3 *db_;
    pthread_t tid_;
    volatile int exit_;
};