cmake_minimum_required(VERSION 3.5)
project(TTchat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++11")
set(SOURCE_FILES server.cpp Monitor.h protocol.h Client.h ClientMonitor.h Conversation.h flp/flp.cpp flp/flp.h flp/flp_crypto.h Connection.h Message.h)

add_executable(TTchat ${SOURCE_FILES})