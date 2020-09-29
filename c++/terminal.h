#ifndef INTELLIPAYTERMINAL_TERMINAL_H
#define INTELLIPAYTERMINAL_TERMINAL_H

#include <string>
#include "nlohmann/json.hpp"
#include <fcntl.h>
#include <termios.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "serial/serial.h"
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace nlohmann;

extern string intellipay_terminal_type;
extern int intellipay_port;
extern string intellipay_serial_port;

serial::Serial stty;

extern bool init(string, int, int, int, bool, bool, string, int);
extern json process(string, json);
extern json sale(float);
extern json refund(float);

#endif //INTELLIPAYTERMINAL_TERMINAL_H
