if(EXISTS CMakeLists.txt)
	message(FATAL_ERROR
		"Looks like you are trying to run cmake from the base sysdig source directory.\n"
		"** RUNNING CMAKE FROM THE BASE SYSDIG DIRECTORY WILL NOT WORK **\n"
		"To Fix:\n"
		" 1. Remove the CMakeCache.txt file in this directory. ex: rm CMakeCache.txt\n"
		" 2. Create a build directory from here. ex: mkdir build\n"
		" 3. cd into that directory. ex: cd build\n"
		" 4. Run cmake from the build directory. ex: cmake ..\n"
		" 5. Run make from the build directory. ex: make\n"
		"Full paste-able example:\n"
		"( rm -f CMakeCache.txt; mkdir build; cd build; cmake ..; make )\n")
endif()

cmake_minimum_required(VERSION 3.4)

project(an3tp VERSION 1.0.0.0)
message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
message("PROJECT_VERSION = ${PROJECT_VERSION}")
message("PROJECT_VERSION_MAJOR = ${PROJECT_VERSION_MAJOR}")
message("PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
message("PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")
message("PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}")


if(NOT CMAKE_BUILD_TYPE)
    SET(CMAKE_BUILD_TYPE Release)
    #SET(CMAKE_BUILD_TYPE Debug)
endif()
message("${CMAKE_PROJECT_NAME} BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

#set(PACKAGE_NAME "an3tp")
#add_definitions(-DPLATFORM_NAME="${CMAKE_SYSTEM_NAME}")

set(DEPS_INCLUDE_DIC "${PROJECT_SOURCE_DIR}/deps/include")
set(UV_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/libuv-v1.37.0/include")
#set(CURL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/libcurl-v7480/include/curl")
#set(VRL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/libvrl")
set(UV_LIB "${PROJECT_SOURCE_DIR}/deps/libuv-v1.37.0/lib/libuv_a.a")
#set(CURL_LIB "${PROJECT_SOURCE_DIR}/deps/libcurl-v7480/lib/libcurl.a")
#comp
#set(SSL_LIB "${PROJECT_SOURCE_DIR}/deps/libcurl-v7480/lib/libssl.a")
#set(CRYPTO_LIB "${PROJECT_SOURCE_DIR}/deps/libcurl-v7480/lib/libcrypto.a")
#set(SSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/libcurl-v7480/include")
set(HIREDIS_INCLUDE "${PROJECT_SOURCE_DIR}/deps/hiredis/include")
set(HIREDIS_LIB "${PROJECT_SOURCE_DIR}/deps/hiredis/lib/libhiredis_static.a")
set(NANOMSG_INCLUDE "${PROJECT_SOURCE_DIR}/deps/libnanomsg/include")
set(NANOMSG_LIB "${PROJECT_SOURCE_DIR}/deps/libnanomsg/lib/libnanomsg.a")


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(TCMALLOC_LIB "${PROJECT_SOURCE_DIR}/deps/libtcmalloc/lib/libtcmalloc.a")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
	set(TCMALLOC_LIB "${PROJECT_SOURCE_DIR}/deps/libtcmalloc/lib/libtcmalloc_minimal.a")
endif()

set(TCMALLOC_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/libtcmalloc/include")

#set(SSL_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/deps/openssl_1_1_1g/include")
#set(SSL_LIB "${PROJECT_SOURCE_DIR}/deps/openssl_1_1_1g/lib/libssl.a")
#set(CRYPTO_LIB "${PROJECT_SOURCE_DIR}/deps/openssl_1_1_1g/lib/libcrypto.a")

message("UV_INCLUDE_DIR = ${UV_INCLUDE_DIR}")
#message("CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")
#message("VRL_INCLUDE_DIR = ${VRL_INCLUDE_DIR}")
message("UV_LIB = ${UV_LIB}")
#message("CURL_LIB = ${CURL_LIB}")
message("TCMALLOC_LIB = ${TCMALLOC_LIB}")
message("HIREDIS_INCLUDE = ${HIREDIS_INCLUDE}")
message("HIREDIS_LIB = ${HIREDIS_LIB}")
message("NANOMSG_INCLUDE = ${NANOMSG_INCLUDE}")
message("NANOMSG_LIB = ${NANOMSG_LIB}")


if(NOT WIN32)
	set(an3tp_DEBUG_FLAGS "-DDEBUG -fno-omit-frame-pointer")
	set(an3tp_RELEASE_FLAGS "-DNDEBUG")

	#c11 / c++11 std
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -ffunction-sections -fdata-sections")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ffunction-sections -fdata-sections")

	#debug
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -g -ggdb ${an3tp_DEBUG_FLAGS}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -Wall -g -ggdb ${an3tp_DEBUG_FLAGS}")

	#release
	#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Wall -g -fno-strict-aliasing ${an3tp_RELEASE_FLAGS}")
	#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -g -fno-strict-aliasing ${an3tp_RELEASE_FLAGS}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -Wall -fno-strict-aliasing ${an3tp_RELEASE_FLAGS}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -fno-strict-aliasing ${an3tp_RELEASE_FLAGS}")

	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -Wl,--gc-sections")
	#kernal
	#if(CMAKE_SYSTEM_NAME MATCHES "Linux")
		#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
			#set(KBUILD_FLAGS "${SYSDIG_DEBUG_FLAGS} ${SYSDIG_FEATURE_FLAGS}")
		#else()
			#set(KBUILD_FLAGS "${SYSDIG_FEATURE_FLAGS}")
		#endif()
	#endif()

else()

	set(an3tp_FLAGS_WIN "-D_CRT_SECURE_NO_WARNINGS -DWIN32 /EHsc /W3 /Zi")
	set(an3tp_FLAGS_WIN_DEBUG "/MTd /Od")
	set(an3tp_FLAGS_WIN_RELEASE "/MT")

	set(CMAKE_C_FLAGS "${an3tp_FLAGS_WIN}")
	set(CMAKE_CXX_FLAGS "${an3tp_FLAGS_WIN}")

	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${an3tp_FLAGS_WIN_DEBUG}")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${an3tp_FLAGS_WIN_DEBUG}")

	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${an3tp_FLAGS_WIN_RELEASE}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${an3tp_FLAGS_WIN_RELEASE}")

endif()

#add_subdirectory(liban3tp)
add_subdirectory(an3tp1)