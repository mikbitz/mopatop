cmake_minimum_required(VERSION 3.0)

project(testagent)

add_executable(testagent main.cpp)

install(TARGETS testagent RUNTIME DESTINATION bin)
