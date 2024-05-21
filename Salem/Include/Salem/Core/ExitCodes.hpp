#pragma once

namespace salem::EXIT {

constexpr int SUCCESS = 0;
constexpr int CRITICAL = 1;
constexpr int LOG_COUNTER_PAST_SWITCH = 2;
constexpr int LOG_COUNTER_CRITICAL_CASE = 3;
constexpr int CLI_HELP_ARG_USED = 4;
constexpr int CLI_MISSING_SRC_FILE = 5;

}
