cmake_minimum_required(VERSION 3.5)
project(TTchat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")

set(SOURCE_FILES main.cpp monitor.h monitor.cpp protocol.cpp protocol.h Client.cpp Client.h)
add_executable(TTchat ${SOURCE_FILES} monitor.h monitor.cpp)