cmake_minimum_required(VERSION 3.13)
project(glwr)

option(LINK "Generates a link to the Khronos site" ON)
option(BRIEF "Generates a short description about the function" ON)
option(VERSION "Generates the OpenGL version where this function first appeared" ON)
option(DESCRIPTION "Generates a long description about the function" OFF)
option(EXAMPLES "Generates examples on how to use the function" OFF)
option(NOTES "Generates notes about the function" OFF)
option(PARAMETERS "Generates information about the parameters" ON)
option(ERRORS "Generates errors that the function can generate" ON)
option(ASSOCIATED_GETS "Generates associated gets" OFF)
option(SEE_ALSO "Generates a 'see also' section" OFF)
option(COPYRIGHT "Generates a copyright notice" ON)

option(VERBOSE "Output the function names as they generate" OFF)

set(INCLUDES "")

function(buildoption NAME ENABLED)
	set(temp ${INCLUDES})

	if (${ENABLED})
		string(APPEND temp "1")
		message("enabled: " ${NAME})
	else()
		string(APPEND temp "0")
	endif()

	set(INCLUDES ${temp} PARENT_SCOPE)
endfunction()

buildoption(LINK ${LINK})
buildoption(BRIEF ${BRIEF})
buildoption(VERSION ${VERSION})
buildoption(DESCRIPTION ${DESCRIPTION})
buildoption(EXAMPLES ${EXAMPLES})
buildoption(NOTES ${NOTES})
buildoption(PARAMETERS ${PARAMETERS})
buildoption(ERRORS ${ERRORS})
buildoption(ASSOCIATED_GETS ${ASSOCIATED_GETS})
buildoption(SEE_ALSO ${SEE_ALSO})
buildoption(COPYRIGHT ${COPYRIGHT})

set(CMAKE_CXX_STANDARD 20)

find_package(ctre REQUIRED)
find_package(pugixml REQUIRED)

include(ExternalProject)
ExternalProject_Add(khronos-opengl-refpages
		GIT_REPOSITORY https://github.com/KhronosGroup/OpenGL-Refpages.git
		GIT_TAG master
		SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/opengl-refpages
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND "")

add_executable(glwr-gen generator/generator.cpp generator/gl1.h generator/XmlHelper.h generator/Options.h generator/Refpage.cpp generator/Refpage.h)
add_dependencies(glwr-gen khronos-opengl-refpages)
target_link_libraries(glwr-gen PRIVATE pugixml)

add_custom_target(create-include-directory ALL
		COMMAND ${CMAKE_COMMAND} -E make_directory include/GL/func)

add_custom_command(
		OUTPUT include/GL/glwr.h
		COMMAND ${CMAKE_CURRENT_BINARY_DIR}/glwr-gen include/GL ${INCLUDES} ${VERBOSE}
		DEPENDS glwr-gen create-include-directory)

add_custom_target(glwr-run ALL DEPENDS include/GL/glwr.h)

add_library(glwr INTERFACE)
add_dependencies(glwr glwr-run)

include(GNUInstallDirs)

install(TARGETS glwr
		EXPORT glwrConfig)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/GL
		DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT glwrConfig DESTINATION share/glwr/cmake)
export(TARGETS glwr FILE glwrConfig.cmake)
