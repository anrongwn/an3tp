#pragma once

#include "spdlog//async.h"
#include "spdlog//fmt/fmt.h"
#include "spdlog/fmt/bin_to_hex.h"
//#include "spdlog/fmt/bundled/printf.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/spdlog.h"
#include <memory>

//日志操作类
namespace anlog {
using anlogger = std::shared_ptr<spdlog::logger>;
extern anlogger &getlogger();
} // namespace anlog

/*
#define debug_log(log)
#define info_log(log)
#define error_log(log)
*/

#define debug_log(log) (anlog::getlogger()->debug(log))
#define info_log(log) (anlog::getlogger()->info(log))
#define error_log(log) (anlog::getlogger()->error(log))
