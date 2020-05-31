cmake_minimum_required(VERSION 3.16)
project(glwr)

SET(CMAKE_CXX_STANDARD 17)

include(ExternalProject)
ExternalProject_Add(khronos-opengl-refpages
		GIT_REPOSITORY https://github.com/KhronosGroup/OpenGL-Refpages.git
		GIT_TAG master
		SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/opengl-refpages
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND "")

add_executable(glwr-gen generator.cpp util.h gl1.h)
add_dependencies(glwr-gen khronos-opengl-refpages)
target_link_libraries(glwr-gen PRIVATE curl)
