#include "an_nn.h"
#include "CJsonObject.hpp"
#include <assert.h>

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

an_data_producer::an_data_producer() : db_(nullptr), tid_(0), exit_(0) {}

an_data_producer::~an_data_producer() {
    if (db_) {
        sqlite3_close(db_);
    }

    if (tid_) {
        pthread_join(tid_, nullptr);
    }
}

sqlite3 *an_data_producer::open_db(const char *dbname) {
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

static int g_row = 0;
int an_data_producer::data_query_cb(void *context, int colcount, char **colvalues, char **colnames) {
    int rc = 0;

    int *p_socker = (int *)context;
    assert(p_socker != nullptr);

    fprintf(stdout, "data_query_cb [%d] \n", ++g_row);

    for (int i = 0; i < colcount; ++i) {
        rc = nn_send(*p_socker, colvalues[i], strlen(colvalues[i]) + 1, 0);
        if (-1 == rc) {
            fprintf(stderr, "nn_send [%s] failed(%d): %s\n", colvalues[i], rc, nn_strerror(rc));
            break;
        }
    }

    return 0;
}

void *an_data_producer::produce(void *args) {
    int rc = 0;

    g_row = 0;
    an_data_producer *pthis = (an_data_producer *)args;
    assert(nullptr != args);

    int p_socker = nn_socket(AF_SP, NN_PUSH);
    assert(p_socker >= 0);

    int eid = nn_bind(p_socker, url);
    assert(eid >= 0);

    char sql_select[256] = {0};
    sprintf(sql_select, "select DATA from Backup order by ID limit 0, %d ;", 10000);

    char *zErrMsg = nullptr;
    rc = sqlite3_exec(pthis->db_, sql_select, an_data_producer::data_query_cb, (void *)&p_socker, &zErrMsg);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "sqliteSelect2 SQL %s error: %s(%d)\n", sql_select, zErrMsg, rc);
        sqlite3_free(zErrMsg);

    } else {
    }
    fprintf(stdout, "an_data_producer::produce.\n");
    return (void *)0;
}

int an_data_producer::start(const char *dbname) {
    int rc = 0;

    db_ = this->open_db(dbname);
    if (db_) {
        rc = pthread_create(&tid_, nullptr, an_data_producer::produce, (void *)this);
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

    int eid = nn_connect(c_socker, url);
    assert(eid >= 0);

    int count = 0;
    while (!pthis->exit_) {
        char *rx_msg = nullptr;
        rc = nn_recv(c_socker, &rx_msg, NN_MSG, /*NN_DONTWAIT*/ 0);
        if (rc > 0) {
            // inser
            fprintf(stdout, "%d - nn_recv : %s, %ld\n", ++count, rx_msg, strlen(rx_msg));

            nn_freemsg(rx_msg);

            sleep(2);
        } else {
            fprintf(stderr, "nn_recv [%d] failed(%d): %s\n", c_socker, rc, nn_strerror(rc));
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