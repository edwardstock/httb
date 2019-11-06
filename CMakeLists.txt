cmake_minimum_required(VERSION 3.0)
project(httb
        VERSION 0.4.0
        DESCRIPTION "Simple C++ Boost HTTP client"
        HOMEPAGE_URL "https://github.com/edwardstock/httb"
        LANGUAGES CXX
        )

execute_process(
	COMMAND bash -c "echo ${CMAKE_PROJECT_VERSION} > ${CMAKE_CURRENT_SOURCE_DIR}/version"
)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/modules)
include(ConanInit)

add_conan_remote(bincrafters https://api.bintray.com/conan/bincrafters/public-conan)
add_conan_remote(scatter https://api.bintray.com/conan/edwardstock/scatter)
conan_init()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "postfix")

option(WITH_TESTS "Enable tests" OFF)

set(HEADERS
    include/httb/helpers.hpp
    include/httb/httb.h
    include/httb/io_container.h
    include/httb/defs.h
    include/httb/body.h
    include/httb/body_multipart.h
    include/httb/body_form_urlencoded.h
    include/httb/body_string.h
    include/httb/timer.h
    include/httb/async_session.h
    include/httb/types.h
    )

set(SOURCES
    src/client.cpp
    src/request.cpp
    src/response.cpp
    src/io_container.cpp
    src/body_multipart.cpp
    src/body_form_urlencoded.cpp
    src/timer.cpp
    src/async_session.cpp
    )

add_library(${PROJECT_NAME} STATIC ${SOURCES})
target_compile_definitions(${PROJECT_NAME} PUBLIC -DHTTB_VERSION="${CMAKE_PROJECT_VERSION}")

target_compile_options(${PROJECT_NAME} PUBLIC -fPIC -pedantic -Wall -Wsign-compare -Wunused-parameter)

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::toolboxpp)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::OpenSSL)

if (HTTB_TEST)
	add_definitions(-DTEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set(PROJECT_NAME_TEST ${PROJECT_NAME}-test)

	execute_process(
		COMMAND "chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/tests/mock/run-server.sh"
		ERROR_VARIABLE MK_EXEC_TEST_ERR
	)

	if (MK_EXEC_TEST_ERR)
		message(FATAL_ERROR ${MK_EXEC_TEST_ERR})
	endif ()

	add_executable(${PROJECT_NAME_TEST}
	               tests/HttpClientTest.cpp
	               tests/HttpClientNoLocalTest.cpp
	               )
	target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_compile_options(${PROJECT_NAME_TEST} PUBLIC -Wno-unused-parameter)
	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})
	target_link_libraries(${PROJECT_NAME_TEST} CONAN_PKG::gtest)

	if (HTTB_COVERAGE)
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

include(modules/install.cmake)
