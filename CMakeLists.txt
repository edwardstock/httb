cmake_minimum_required(VERSION 3.0)
project(httb
        VERSION 0.1.0
        DESCRIPTION "Simple C++ Boost HTTP client"
        HOMEPAGE_URL "https://github.com/edwardstock/httb"
        LANGUAGES CXX
        )

include(cmake-modules/conan.cmake)

conan_add_remote(
	NAME bincrafters
	URL https://api.bintray.com/conan/bincrafters/public-conan
)
conan_cmake_run(
	CONANFILE conanfile.txt
	BUILD missing
	BASIC_SETUP
)

# CONAN
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
	include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
	conan_basic_setup(TARGETS)
else ()
	message(WARNING "The file ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "postfix")

option(WITH_TESTS "Enable tests" OFF)

### BOOST
#if (NOT BOOST_ROOT)
#	set(BOOST_ROOT ${CONAN_BOOST_ROOT})
#endif ()
set(Boost_DEBUG OFF)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)

#Toolbox
if (NOT TARGET toolboxpp)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/toolboxpp)
endif ()

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
target_compile_definitions(${PROJECT_NAME} PUBLIC -DHTTB_VERSION="${CMAKE_PROJECT_VERSION}")

include_directories(cmake-modules/PlatformDetector.cmake)
target_compile_options(${PROJECT_NAME} PUBLIC -fPIC -pedantic -Wall -Wsign-compare -Wunused-parameter)

target_include_directories(${PROJECT_NAME} PUBLIC include)

#target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
#target_include_directories(${PROJECT_NAME} PUBLIC ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_beast)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_regex)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_asio)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_system)
target_link_libraries(${PROJECT_NAME} CONAN_PKG::boost_filesystem)

target_link_libraries(${PROJECT_NAME} toolboxpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libs/toolboxpp/include)

target_link_libraries(${PROJECT_NAME} CONAN_PKG::OpenSSL)


add_executable(${PROJECT_NAME}-bin main.cpp)
target_link_libraries(${PROJECT_NAME}-bin ${PROJECT_NAME})

if (WITH_TESTS)
	add_definitions(-DTEST_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests")
	set(PROJECT_NAME_TEST ${PROJECT_NAME}-test)

	execute_process(
		COMMAND "chmod +x ${CMAKE_CURRENT_SOURCE_DIR}/tests/mock/run-server.sh"
		ERROR_VARIABLE MK_EXEC_TEST_ERR
	)

	if (MK_EXEC_TEST_ERR)
		message(FATAL_ERROR ${MK_EXEC_TEST_ERR})
	endif ()

	if (WIN32)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
	elseif (APPLE)
		add_definitions(-DGTEST_USE_OWN_TR1_TUPLE)
		add_definitions(-D__GLIBCXX__)
	endif ()

	add_executable(${PROJECT_NAME_TEST} tests/HttpClientTest.cpp)
	target_compile_options(${PROJECT_NAME_TEST} PUBLIC -Wno-unused-parameter)

	target_link_libraries(${PROJECT_NAME_TEST} ${PROJECT_NAME})

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
	target_include_directories(${PROJECT_NAME_TEST} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

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
