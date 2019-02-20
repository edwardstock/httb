cmake_minimum_required(VERSION 3.0)
project(httb
        VERSION 1.0.0
        DESCRIPTION "Simple C++ Boost HTTP client"
        HOMEPAGE_URL "https://github.com/edwardstock/httb"
        LANGUAGES CXX
        )

add_definitions(-DHTTB_VERSION="${CMAKE_PROJECT_VERSION}")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "postfix")

option(WITH_TESTS "Enable tests" OFF)

include_directories(include)

set(SOURCES
    src/client.cpp
    src/request.cpp
    src/response.cpp
    src/helpers.hpp
    include/httb/httb.h
    include/httb/io_container.h
    include/httb/defs.h
    include/httb/body.h
    include/httb/body_serializer.h
    src/io_container.cpp
    include/httb/body_multipart.h
    src/body_multipart.cpp
    include/httb/body_form_urlencoded.h
    src/body_form_urlencoded.cpp
    include/httb/body_string.h
    src/body_string.cpp)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_compile_options(${PROJECT_NAME} PUBLIC -pedantic -Wall -Wsign-compare -Wunused-parameter)

## BOOST
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.54.0 COMPONENTS system thread random filesystem REQUIRED)
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})

#OpenSSL
string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_LOWER)
if (NOT OPENSSL_ROOT_DIR)
	set(OPENSSL_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/openssl/${SYSTEM_LOWER}_${CMAKE_SYSTEM_PROCESSOR}")
endif ()
set(OPENSSL_USE_STATIC_LIBS ON)
find_package(OpenSSL 1.1.0 REQUIRED)

if (NOT TARGET toolboxpp)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/toolboxpp)
endif ()
target_link_libraries(${PROJECT_NAME} toolboxpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/toolboxpp/include)

target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENSSL_INCLUDE_DIR})
message(STATUS "OpenSSL ${OPENSSL_VERSION} (${OPENSSL_LIBRARIES})")


add_executable(${PROJECT_NAME}-bin main.cpp)

target_link_libraries(${PROJECT_NAME}-bin ${PROJECT_NAME})

if (WITH_TESTS)
	add_definitions(-DTEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set(PROJECT_NAME_TEST ${PROJECT_NAME}-test)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/googletest/googletest/include)
	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/googletest/googlemock/include)
	set(GTEST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs/googletest/googletest)

	include_directories(${GTEST_SOURCE_DIR}/include ${GTEST_SOURCE_DIR})
	add_subdirectory(${GTEST_SOURCE_DIR})
	if (WIN32)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
	elseif (APPLE)
		add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
		add_definitions(-D__GLIBCXX__)
	endif ()


	add_executable(${PROJECT_NAME_TEST} ${SERVER_EXEC_SRCS}
	               #               tests/base/TestAuth.cpp
	               tests/HttpClientTest.cpp
	               )
	target_compile_options(${PROJECT_NAME_TEST} PUBLIC -Wno-unused-parameter)

	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
	target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

	target_link_libraries(${PROJECT_NAME_TEST} gtest gtest_main)

	if (WITH_COVERAGE)
		set(COVERAGE_LCOV_EXCLUDES
		    '${CMAKE_CURRENT_SOURCE_DIR}/libs/*'
		    '${CMAKE_CURRENT_SOURCE_DIR}/tests/*'
		    'boost/*'
		    '/usr/include/*'
		    '/usr/local/*'
		    'v1'
		    )
		set(COVERAGE_TARGET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/)
		include(cmake-modules/CodeCoverage.cmake)
		append_coverage_compiler_flags()
		setup_target_for_coverage_lcov(
			NAME ${PROJECT_NAME}-coverage
			EXECUTABLE ${PROJECT_NAME_TEST}
			DEPENDENCIES ${PROJECT_NAME_TEST}
		)
	endif ()
endif ()
