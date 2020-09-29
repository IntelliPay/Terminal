cmake_minimum_required(VERSION 3.17)
project(IntelliPayTerminal)

set(CMAKE_CXX_STANDARD 14)

add_library(IntelliPayTerminal SHARED terminal.cpp terminal.h serial/serial.h serial/v8stdint.h serial/impl/win.h serial/impl/unix.h serial/serial.cpp serial/impl/unix.cpp serial/impl/win.cpp serial/impl/list_ports/list_ports_linux.cpp serial/impl/list_ports/list_ports_osx.cpp serial/impl/list_ports/list_ports_win.cpp)