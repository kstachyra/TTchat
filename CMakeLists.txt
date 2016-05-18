cmake_minimum_required(VERSION 2.8)
project(TTchat)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.old.cpp)

set(PROJECT_LINK_LIBS
		libtomcrypt.a
		-lpthread)

add_executable(TTchat ${SOURCE_FILES})

target_link_libraries(TTchat ${PROJECT_LINK_LIBS})
