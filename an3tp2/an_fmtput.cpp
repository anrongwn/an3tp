#include "an_fmtput.h"

//全局 color stdout
const char *AN_STDOUT_COLOR_NAME = "an_fmt_console";
// auto g_console = spdlog::stdout_color_mt(AN_STDOUT_COLOR_NAME);

std::shared_ptr<spdlog::logger> g_console = spdlog::stdout_color_mt(AN_STDOUT_COLOR_NAME);