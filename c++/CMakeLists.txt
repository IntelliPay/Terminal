cmake_minimum_required(VERSION 3.17)
project(IntelliPayTerminal)

set(CMAKE_CXX_STANDARD 14)

add_library(IntelliPayTerminal SHARED terminal.cpp terminal.h)