cmake_minimum_required(VERSION 3.0)
project(httb
        VERSION 1.0.1
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
add_conan_remote(edwardstock https://api.bintray.com/conan/edwardstock/conan-public)
conan_init()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "postfix")

if (MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2 /MP")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /DEBUG /Od")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
	string(REGEX REPLACE "\\/W3" " " CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	#	add_compile_options(/wd4068)
elseif (MINGW)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -Wno-unknown-pragmas -Wno-shift-count-overflow")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
else ()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fvisibility=hidden -fPIC -Wextra -pedantic -Wno-unknown-pragmas -Wno-attributes -Wno-unused-function -Wno-unused-parameter -Wno-shift-count-overflow")
	set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -D_LIBCPP_DEBUG=1")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif ()

option(ENABLE_TEST "Enable tests" OFF)

set(HTTB_EXPORTING 1)
if (ENABLE_SHARED)
	set(HTTB_SHARED 1)
endif ()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cfg/httb_config.h.in
               ${CMAKE_CURRENT_SOURCE_DIR}/include/httb/httb_config.h)

set(HEADERS
    include/httb/httb_config.h
    include/httb/httb.h
    include/httb/io_container.h
    include/httb/body.h
    include/httb/body_multipart.h
    include/httb/body_form_urlencoded.h
    include/httb/body_string.h
    include/httb/types.h
    src/async_session.h
    src/utils.h
    include/httb/mocker/mock_client.h
    )


set(SOURCES
    ${HEADERS}
    src/client.cpp
    src/request.cpp
    src/response.cpp
    src/io_container.cpp
    src/body_string.cpp
    src/body_multipart.cpp
    src/body_form_urlencoded.cpp
    src/async_session.cpp
    )

if (ENABLE_SHARED)
	add_library(${PROJECT_NAME} SHARED ${SOURCES})
else ()
	add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif ()
target_compile_definitions(${PROJECT_NAME} PUBLIC -DHTTB_VERSION="${CMAKE_PROJECT_VERSION}")

target_include_directories(${PROJECT_NAME} PUBLIC include)

target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::toolbox)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::OpenSSL)

if (ENABLE_TEST)
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
	               tests/main.cpp
	               tests/HttpClientTest.cpp

	               )
	target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})
	target_link_libraries(${PROJECT_NAME_TEST} CONAN_PKG::gtest)

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

include(modules/install.cmake)
