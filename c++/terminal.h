#ifndef INTELLIPAYTERMINAL_TERMINAL_H
#define INTELLIPAYTERMINAL_TERMINAL_H

#include <string>
#include "nlohmann/json.hpp"
#include <fcntl.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <iostream>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

using namespace std;
using namespace nlohmann;
#include "serial/serial.h"

#ifdef IntelliPayTerminal_EXPORTS
#define INTELLIPAY_DLL __declspec(dllexport)
#else
#define INTELLIPAY_DLL __declspec(dllimport)
#endif


namespace IntelliPayTerminal {

    INTELLIPAY_DLL string intellipay_terminal_type;
    INTELLIPAY_DLL int intellipay_port;
    INTELLIPAY_DLL string intellipay_serial_port;

    INTELLIPAY_DLL serial::Serial stty;


    INTELLIPAY_DLL bool
    init(string, string, int, serial::bytesize_t, serial::stopbits_t, serial::flowcontrol_t, bool, int);

    INTELLIPAY_DLL string process(string, float, string);

    INTELLIPAY_DLL string sale(float, string);

    INTELLIPAY_DLL string refund(float, string);

}

#endif //INTELLIPAYTERMINAL_TERMINAL_H
