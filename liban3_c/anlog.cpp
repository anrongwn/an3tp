#include "anlog.h"

#include "uv.h"
#include <algorithm>
#include <iostream>

#include "anNoCopyable.h"
#include <sstream>
#include <string>

#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#define mkdir _mkdir
#define access _access
#define create_log_path create_log_path_w
#define DIR_SEP ('\\')
#elif defined(__GNUC__)
#define MAX_PATH 260

#include <sys/stat.h>
#include <sys/types.h>
#define DIR_SEP ('/')
#define create_log_path create_log_path_x
#endif

static int create_log_path_x(const std::string &path) {
    int r = 0;
    char tmp[MAX_PATH] = {0X00};

    std::size_t len = path.length();
    std::memcpy(tmp, path.c_str(), len);
    if (DIR_SEP != tmp[len - 1]) {
        std::strcat(tmp, "/");
    }

    for (std::size_t i = 1; i <= len; ++i) {
        if (DIR_SEP == tmp[i]) {
            tmp[i] = 0x00;
            if (0 != access(tmp, F_OK)) {
#ifdef __GNUC__
                r = mkdir(tmp, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#elif _MSC_VER
                r = mkdir(tmp);
#endif
                if (-1 == r) {
                    // abort();
                    return r;
                } else {
                    // std::cout << "mkdir " << tmp << " ,=" << r;
                }
            }
            tmp[i] = DIR_SEP;
        }
    }
    return r;
}

struct log_wrapp : public anNoCopyable {
    log_wrapp() {
        std::string logpath("/opt/chisels/log");

        uv_pid_t pid = uv_os_getpid();
        std::string logname("libvrl-");
        logname += std::to_string(pid);

        //
        create_log_path(logpath);

        logpath += R"(/)";
        logpath += logname;
        logpath += R"(.log)";

        /*//是否已启动日志线程池?
        auto tp = spdlog::thread_pool();
        if (!tp) {
            // spdlog::init_thread_pool(65536, 1);
            spdlog::init_thread_pool(8192, 1);
        }

        g_anlog =
            spdlog::daily_logger_mt<spdlog::async_factory>(logname, logpath);
        */
        log_ = spdlog::daily_logger_mt<spdlog::default_factory>(logname, logpath);

        ////毫秒 Milliseconds
        // g_anlog->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^---%L---%$] [%t]
        // %v");

        //微秒 Microseconds
        // g_anlog->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^---%L---%$] [%t]
        // %v");
        log_->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%=8l] [%t] %v");

#ifdef DEBUG
        log_->set_level(spdlog::level::debug);
        // g_anlog->set_level(spdlog::level::info);
#else
        log_->set_level(spdlog::level::debug);
#endif

        log_->flush_on(spdlog::level::debug);
    }

    using anlogger = std::shared_ptr<spdlog::logger>;
    anlogger &getlogger() { return log_; }

  private:
    anlogger log_;
};

//日志初始化
anlog::anlogger &anlog::getlogger() {
    static log_wrapp g_logger; //多线程安全
    return g_logger.getlogger();
}
