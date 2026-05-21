cmake_minimum_required(VERSION 3.20)
project(zadanie1)

#add_compile_options(-lboost_system)
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)
set(CMAKE_CXX_FLAGS "-lboost_program_options -Wall -O2")
set(CMAKE_CXX_STANDARD 20)

add_executable(sikradio-sender sender.cpp sikradio-sender.cpp)

add_executable(sikradio-receiver buffer.cpp receiver.cpp sikradio-receiver.cpp)
