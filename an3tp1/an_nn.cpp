#include "an_nn.h"
#include "CJsonObject.hpp"
#include <assert.h>
#include <chrono>

/*
  此程序为nanomsg多线程一对一单向通信demo。
*/

// inproc 标识用于多线程通信
static const char *url = "inproc://an3tp1";

//发送数据的socket初始化
int send_sock_init(int *sock) {
    *sock = nn_socket(AF_SP, NN_PUSH);
    if (*sock < 0) {
        printf("create send data sock failed\r\n");
        return 1;
    }

    if (nn_bind(*sock, url) < 0) {
        printf("bind send data sock failed\r\n");
        return 1;
    }

    printf("send data socket init success...\r\n");
    return 0;
}

//接收数据的socket初始化
int recieve_sock_init(int *sock) {
    *sock = nn_socket(AF_SP, NN_PULL);
    if (*sock < 0) {
        printf("create recieve data sock failed\r\n");
        return 1;
    }
    if (nn_connect(*sock, url) < 0) {
        printf("connect recieve data sock failed\r\n");
        return 1;
    }
    printf("recieve data socket init success...\r\n");
    return 0;
}

//线程测试
void *thread_test(void *arg) {
    int c_sock;
    if (0 != recieve_sock_init(&c_sock)) {
        return 0;
    }

    while (1) {
        //轮询接收信息
        char *rx_msg = NULL;
        int result = nn_recv(c_sock, &rx_msg, NN_MSG, NN_DONTWAIT);
        if (result > 0) {
            printf("Thread Recieve: %s\r\n", rx_msg);
            nn_freemsg(rx_msg);
        }

        sleep(1);
    }

    return 0;
}

an_data_producer::an_data_producer() : tid_(0) {
    context_.data = this;
    context_.nn_socket = 0;
    context_.db_ = nullptr;
    context_.exit_ = 0;
}

an_data_producer::~an_data_producer() {
    if (context_.db_) {
        sqlite3_close(context_.db_);
    }

    if (tid_) {
        pthread_join(tid_, nullptr);
    }
}

sqlite3 *an_data_producer::open_db(const char *dbname) {
    assert(dbname != nullptr);

    if (context_.db_) {
        sqlite3_close(context_.db_);

        context_.db_ = nullptr;
    }

    int rc = sqlite3_open(dbname, &context_.db_);
    if (rc) {
        fprintf(stderr, "sqlite3_open [%s] database: %s\n", dbname, sqlite3_errmsg(context_.db_));
        return nullptr;
    }

    return context_.db_;
}

static int g_row = 0;
int an_data_producer::data_query_cb(void *context, int colcount, char **colvalues, char **colnames) {
    int rc = 0;

    sql_context_t *ctx = (sql_context_t *)context;
    assert(ctx != nullptr);

    if (ctx->exit_) {
        fprintf(stdout, "data_query_cb exit(%d). \n", ++g_row);
        return 1;
    }

    for (int i = 0; i < colcount; ++i) {
        rc = nn_send(ctx->nn_socket, colvalues[i], strlen(colvalues[i]) + 1, 0 /*NN_DONTWAIT*/);
        if (-1 == rc) {
            int nnerr = nn_errno();
            if (nnerr == EAGAIN) {
                // fprintf(stdout, "data_query_cb [%d] \n", ++g_row);
            } else {
                fprintf(stderr, "nn_send [%d] failed(%d): %s\n", ctx->nn_socket, nnerr, nn_strerror(nnerr));

                return 1;
            }
        } else {
            fprintf(stdout, "data_query_cb [%d], len =%d \n", ++g_row, rc);
        }
    }

    return 0;
}

void *an_data_producer::produce(void *args) {
    int rc = 0;

    g_row = 0;
    sql_context_t *ctx = (sql_context_t *)args;
    assert(nullptr != ctx);

    ctx->nn_socket = nn_socket(AF_SP, NN_PUSH);
    assert(ctx->nn_socket >= 0);

    int sndbuf_size = 1024 * 256; // 10M
    rc = nn_setsockopt(ctx->nn_socket, NN_SOL_SOCKET, NN_SNDBUF, &sndbuf_size, sizeof(sndbuf_size));
    assert(rc >= 0);

    sndbuf_size = 0;
    size_t tmp = sizeof(sndbuf_size);
    rc = nn_getsockopt(ctx->nn_socket, NN_SOL_SOCKET, NN_SNDBUF, &sndbuf_size, &tmp);
    assert(rc >= 0);
    fprintf(stdout, "nn_getsockopt(%d) sndbuf=%d \n", ctx->nn_socket, sndbuf_size);

    int eid = nn_bind(ctx->nn_socket, url);
    assert(eid >= 0);

    auto start = std::chrono::system_clock::now();

    char sql_select[256] = {0};
    sprintf(sql_select, "select DATA from Backup order by ID limit 0, %d ;", 50001);

    char *zErrMsg = nullptr;
    rc = sqlite3_exec(ctx->db_, sql_select, an_data_producer::data_query_cb, (void *)ctx, &zErrMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "sqlite Select2 SQL %s error: %s(%d)\n", sql_select, zErrMsg, rc);
        sqlite3_free(zErrMsg);

    } else {
    }

    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> diff = end - start;
    fprintf(stdout, "an_data_producer::produce exit. elapsed-time:%lf \n", diff.count());

    return (void *)0;
}

int an_data_producer::start(const char *dbname) {
    int rc = 0;

    context_.db_ = this->open_db(dbname);
    if (context_.db_) {
        rc = pthread_create(&tid_, nullptr, an_data_producer::produce, (void *)&this->context_);
        if (rc) {
            tid_ = 0;
            fprintf(stderr, "pthread_create failed(%d) %s\n", rc, strerror(rc));
            return rc;
        }

        usleep(100000);
    }

    return rc;
}

//
an_data_consumer::an_data_consumer() : db_(nullptr), tid_(0), exit_(0) {}

an_data_consumer::~an_data_consumer() {
    if (db_) {
        sqlite3_close(db_);
    }

    if (tid_) {
        pthread_join(tid_, nullptr);
    }
}

sqlite3 *an_data_consumer::open_db(const char *dbname) {
    assert(dbname != nullptr);

    if (db_) {
        sqlite3_close(db_);

        db_ = nullptr;
    }

    int rc = sqlite3_open(dbname, &db_);
    if (rc) {
        fprintf(stderr, "sqlite3_open [%s] database: %s\n", dbname, sqlite3_errmsg(db_));
        return nullptr;
    }

    return db_;
}

void *an_data_consumer::consume(void *args) {
    int rc = 0;

    an_data_consumer *pthis = (an_data_consumer *)args;
    assert(pthis != nullptr);

    int c_socker = nn_socket(AF_SP, NN_PULL);
    assert(c_socker >= 0);

    int rcvbuf_size = 1024 * 256;
    rc = nn_setsockopt(c_socker, NN_SOL_SOCKET, NN_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size));
    assert(rc >= 0);

    rcvbuf_size = 0;
    size_t tmp = sizeof(rcvbuf_size);
    rc = nn_getsockopt(c_socker, NN_SOL_SOCKET, NN_RCVBUF, &rcvbuf_size, &tmp);
    assert(rc >= 0);
    fprintf(stdout, "nn_getsockopt(%d) rcvbuf=%d \n", c_socker, rcvbuf_size);

    int eid = nn_connect(c_socker, url);
    assert(eid >= 0);

    int count = 0;
    while (!pthis->exit_) {
        char *rx_msg = nullptr;
        rc = nn_recv(c_socker, &rx_msg, NN_MSG, 0 /*NN_DONTWAIT*/);
        if (rc > 0) {
            // inser
            // fprintf(stdout, "%d - nn_recv : %s, %ld\n", ++count, rx_msg, strlen(rx_msg));
            fprintf(stdout, "%d - nn_recv : %ld\n", ++count, strlen(rx_msg));

            nn_freemsg(rx_msg);

            // sleep(2);
        } else {
            fprintf(stderr, "nn_recv [%d] failed(%d): %s\n", c_socker, nn_errno(), nn_strerror(nn_errno()));
            break;
        }
    }

    fprintf(stdout, "an_data_consumer::consume exit.\n");
    return (void *)0;
}
int an_data_consumer::start(const char *dbname) {
    int rc = 0;

    db_ = this->open_db(dbname);
    if (db_) {
        rc = pthread_create(&tid_, nullptr, an_data_consumer::consume, (void *)this);
        if (rc) {
            tid_ = 0;
            fprintf(stderr, "pthread_create failed(%d) %s\n", rc, strerror(rc));
            return rc;
        }

        // usleep(100000);
    }

    return rc;
}