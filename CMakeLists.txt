cmake_minimum_required(VERSION 3.16)
project(mgl)

SET(CMAKE_CXX_STANDARD 17)

add_executable(mglgen generator.cpp download.h util.h gl1.h)
target_link_libraries(mglgen PRIVATE curl)
