cmake_minimum_required(VERSION 3.4)

#project(libvrl VERSION 1.0.0.0)
#message("CMAKE_PROJECT_NAME = ${CMAKE_PROJECT_NAME}")
#message("PROJECT_VERSION = ${PROJECT_VERSION}")
#message("PROJECT_VERSION_MAJOR    = ${PROJECT_VERSION_MAJOR}")
#message("PROJECT_VERSION_MINOR = ${PROJECT_VERSION_MINOR}")
#message("PROJECT_VERSION_PATCH = ${PROJECT_VERSION_PATCH}")
#message("PROJECT_VERSION_TWEAK = ${PROJECT_VERSION_TWEAK}")
message("libvrl BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message("CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
message("CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")

message("CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
message("CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")


set(VRL_TARGET_NAME vrl)
add_definitions(-D BUILDING_VRL_SHARED)

include_directories(${DEPS_INCLUDE_DIC})
include_directories(${UV_INCLUDE_DIR})
include_directories(${CURL_INCLUDE_DIR})
include_directories(${SSL_INCLUDE_DIR})
include_directories(${TCMALLOC_INCLUDE_DIR})

include_directories(.)

if (NOT WIN32)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -shared -fPIC ")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared -fPIC -static-libstdc++ ")


    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )
    set( CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free" )
else()
endif()

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

aux_source_directory(. SRCS_DIR)

#link_libraries(${SSL_LIB} ${CRYPTO_LIB})
link_libraries(${UV_LIB} ${CURL_LIB})
link_libraries(${TCMALLOC_LIB})



#add_library(${VRL_TARGET_NAME} STATIC ${SRCS_DIR})
add_library(${VRL_TARGET_NAME} SHARED ${SRCS_DIR})

if(NOT WIN32)
    set_target_properties(${VRL_TARGET_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)
    set_target_properties(${VRL_TARGET_NAME} PROPERTIES OUTPUT_NAME ${VRL_TARGET_NAME})
    set_target_properties (${VRL_TARGET_NAME} PROPERTIES CLEAN_DIRECT_OUTPUT 1)
    set_target_properties(${VRL_TARGET_NAME} PROPERTIES C_VISIBILITY_PRESET hidden)
    set_target_properties(${VRL_TARGET_NAME} PROPERTIES CXX_VISIBILITY_PRESET hidden)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_libraries(${VRL_TARGET_NAME} dl pthread rt z unwind ${SSL_LIB} -Wl,--whole-archive ${CRYPTO_LIB} -Wl,--no-whole-archive)
    else()
        target_link_libraries(${VRL_TARGET_NAME} dl pthread rt z ${SSL_LIB} -Wl,--whole-archive ${CRYPTO_LIB} -Wl,--no-whole-archive)
    endif()

    
    #target_link_libraries(${VRL_TARGET_NAME} pthread dl rt z ssl crypto ${UV_LIB} ${CURL_LIB} ${TCMALLOC_LIB})
    #target_link_libraries(${VRL_TARGET_NAME} ssl crypto dl pthread rt z unwind)
    #target_link_libraries(${VRL_TARGET_NAME} ${CRYPTO_LIB} ${SSL_LIB} ${UV_LIB} ${CURL_LIB} ${TCMALLOC_LIB}  pthread dl rt z)
else()
endif()