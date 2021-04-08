cmake_minimum_required(VERSION 3.13) # see spec and target OS

project(risking VERSION 0.1)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug" CACHE STRING
		"Default build type: Debug" FORCE)
	message(NOTICE "-- CMAKE_BUILD_TYPE not provided, set to ${CMAKE_BUILD_TYPE}")
endif()

# Build test flag
option(BUILD_TEST "Building test" ON)

# directories
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules")
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INC_DIR "${PROJECT_SOURCE_DIR}/include")
set(TES_DIR "${PROJECT_SOURCE_DIR}/tests")

add_executable(risking)
target_sources(risking PUBLIC "${SRC_DIR}/main.cpp")

# configuration file
option(DUMP_SSL_SECRETS "DUMP SSL secrets for debugging" OFF)
if (DUMP_SSL_SECRETS)
	message(WARNING "-- Dumping SSL secrets is unsecure")
endif()
configure_file("${SRC_DIR}/configuration.h.in" "${INC_DIR}/configuration.h")

# Main library, maybe split for faster compilation ??
add_library(risking_lib)
file(GLOB_RECURSE SRC_LIST ${SRC_DIR}/*.cpp)
list(REMOVE_ITEM SRC_LIST "${SRC_DIR}/main.cpp" "${SRC_DIR}/configuration.h.in")
target_sources(risking_lib PRIVATE ${SRC_LIST})
target_include_directories(risking_lib PUBLIC ${INC_DIR})

target_link_libraries(risking risking_lib)

# C++ standard, see spec
target_compile_features(risking_lib PUBLIC cxx_std_17)
set_target_properties(risking_lib risking PROPERTIES 
	CXX_EXTENSIONS NO
	CXX_STANDARD_REQUIRED YES)

# Flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG -fno-inline")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

# Boost
find_package(Boost 1.67 REQUIRED COMPONENTS
	system thread regex unit_test_framework log)
target_include_directories(risking_lib PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(risking_lib PUBLIC Boost::system Boost::thread Boost::regex Boost::log)

# OpenSSL
find_package(OpenSSL REQUIRED)
target_include_directories(risking_lib PUBLIC ${OPENSSL_INCLUDE_DIR})
target_link_libraries(risking_lib PRIVATE OpenSSL::SSL)

# Tests
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND BUILD_TEST)
	include(CTest)
endif()

if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND COVERAGE)
	include(CodeCoverage)
endif()

enable_testing()

if((((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR RISKING_BUILD_TESTING) AND BUILD_TESTING)) AND BUILD_TEST)
	include("${TES_DIR}/boost.cmake")
endif()
