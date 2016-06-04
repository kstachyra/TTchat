cmake_minimum_required(VERSION 3.2)
project(TTchat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
set(SOURCE_FILES server.cpp Monitor.h Client.h ClientMonitor.h Chatroom.h flp/flp.cpp flp/flp.h flp/flp_crypto.h flp/flp_crypto.cpp flp/flp_config.h Message.h MySemaphore.cpp MySemaphore.h)

add_executable(TTchat ${SOURCE_FILES})