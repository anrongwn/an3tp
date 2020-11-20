cmake_minimum_required(VERSION 3.4)

#project(anredistp VERSION 1.0.0.0)
#message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
#message("PROJECT_VERSION = ${PROJECT_VERSION}")
#message("PROJECT_VERSION_MAJOR    = ${PROJECT_VERSION_MAJOR}")
#message("PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
#message("PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")
#message("PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}")
set(anredistp_TARGET_NAME anredistp)

include_directories(${DEPS_INCLUDE_DIC})
include_directories(${UV_INCLUDE_DIR})
#include_directories(${CURL_INCLUDE_DIR})
include_directories(${VRL_INCLUDE_DIR})
include_directories(${TCMALLOC_INCLUDE_DIR})
include_directories(${HIREDIS_INCLUDE})
include_directories(.)

message("anredistp UV_INCLUDE_DIR = ${UV_INCLUDE_DIR}")
message("anredistp CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")
message("anredistp HIREDIS_INCLUDE = ${HIREDIS_INCLUDE}")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_SKIP_BUILD_RPATH true)
aux_source_directory(. SRCS_DIR)

#link_libraries(${UV_LIB} ${CURL_LIB})
#link_libraries(${TCMALLOC_LIB})
add_executable(${anredistp_TARGET_NAME} ${SRCS_DIR})

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -fPIC -shared")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -shared")
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )
    set( CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )

    
    set_target_properties(${anredistp_TARGET_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

    #target_link_libraries(anredistp pthread dl rt z crypto ssl vrl)
    target_link_libraries(${anredistp_TARGET_NAME} ${HIREDIS_LIB} )  
    
else()
    target_link_libraries(${anredistp_TARGET_NAME} ${UV_LIB} ${CURL_LIB} ${VRL_LIB})
endif()
