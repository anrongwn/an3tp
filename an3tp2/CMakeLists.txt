cmake_minimum_required(VERSION 3.4)

#project(an3tp2 VERSION 1.0.0.0)
#message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
#message("PROJECT_VERSION = ${PROJECT_VERSION}")
#message("PROJECT_VERSION_MAJOR    = ${PROJECT_VERSION_MAJOR}")
#message("PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
#message("PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")
#message("PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}")
set(an3tp2_TARGET_NAME an3tp2)

include_directories(${DEPS_INCLUDE_DIC})
include_directories(${UV_INCLUDE_DIR})
#include_directories(${CURL_INCLUDE_DIR})
include_directories(${VRL_INCLUDE_DIR})
include_directories(${TCMALLOC_INCLUDE_DIR})
include_directories(${HIREDIS_INCLUDE})
include_directories(${NNG_INCLUDE})
include_directories(${CJsonObject_INCLUDE})
include_directories(${SQLITE3_INCLUDE})
include_directories(.)

message("an3tp2 UV_INCLUDE_DIR = ${UV_INCLUDE_DIR}")
message("an3tp2 CURL_INCLUDE_DIR = ${CURL_INCLUDE_DIR}")
message("an3tp2 HIREDIS_INCLUDE = ${HIREDIS_INCLUDE}")
message("an3tp2 NNG_INCLUDE = ${NNG_INCLUDE}")


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_SKIP_BUILD_RPATH true)
aux_source_directory(. SRCS_DIR)
aux_source_directory(../deps/CJsonObject SRCS_DIR)
aux_source_directory(../deps/sqlite3.33 SRCS_DIR)

#link_libraries(${UV_LIB} ${CURL_LIB})
#link_libraries(${TCMALLOC_LIB})
add_executable(${an3tp2_TARGET_NAME} ${SRCS_DIR})

if (NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -fPIC -shared")
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -shared")
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )
    set( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )

    
    set_target_properties(${an3tp2_TARGET_NAME} PROPERTIES LINK_FLAGS "-Wl,-rpath,./")

    #target_link_libraries(an3tp2 pthread dl rt z crypto ssl vrl)

    #link libnanomsg.a
    #target_link_libraries(${an3tp2_TARGET_NAME} dl rt -pthread anl nsl ${HIREDIS_LIB} ${NANOMSG_LIB} )
    
    #link libnng.a
    target_link_libraries(${an3tp2_TARGET_NAME} dl rt -pthread nsl ${HIREDIS_LIB} ${NNG_LIB} )  
    
else()
    target_link_libraries(${an3tp2_TARGET_NAME} ${UV_LIB} ${CURL_LIB} ${VRL_LIB})
endif()
