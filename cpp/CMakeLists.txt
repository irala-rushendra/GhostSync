#author : irala rushendra
#email  : iralarushendra@gmail.com

cmake_minimum_required(VERSION 3.10.2)
project(GhostSyncEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Locate the NCNN pre-compiled mobile libraries
# (Assume ncnn is placed in cpp/ncnn-mobile)
set(NCNN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ncnn-mobile)
include_directories(${NCNN_DIR}/include/ncnn)

set(GHOSTSYNC_SRC
    engine.cpp
    model_runner.cpp
    jsi_bindings.cpp
)

add_library(ghostsync SHARED ${GHOSTSYNC_SRC})

if(ANDROID)
    find_library(log-lib log)
    find_library(jnigraphics-lib jnigraphics) # Needed if we convert Android Bitmaps
    target_link_libraries(ghostsync
        ${NCNN_DIR}/lib/libncnn.a
        ${log-lib}
        ${jnigraphics-lib}
    )
elseif(APPLE)
    # iOS specific linking for NCNN
    target_link_libraries(ghostsync
        ${NCNN_DIR}/lib/libncnn.a
        "-framework Foundation"
        "-framework Accelerate" # Required by NCNN on iOS
    )
endif()