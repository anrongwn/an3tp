#pragma once

#include "spdlog//async.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/fmt/fmt.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

extern const char *AN_STDOUT_COLOR_NAME;

extern std::shared_ptr<spdlog::logger> g_console;

#define INIT_AN_STDOUT(t) spdlog::get(AN_STDOUT_COLOR_NAME)->set_pattern(t);
#define AN_STDOUT_INFO(t) spdlog::get(AN_STDOUT_COLOR_NAME)->info(t);
#define AN_STDOUT_ERROR(t) spdlog::get(AN_STDOUT_COLOR_NAME)->error(t);
#define AN_STDOUT_WARN(t) spdlog::get(AN_STDOUT_COLOR_NAME)->warn(t);
