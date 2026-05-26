cmake_minimum_required(VERSION 3.10.2)
project(ghostsync)

# 1. Set paths to your React Native JSI headers
set(REACT_NATIVE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../node_modules/react-native)
set(FOLLY_DIR ${REACT_NATIVE_DIR}/ReactCommon/folly)

# 2. Set path to your pre-compiled Android NCNN libraries (You will need to download the Android NCNN release and put it in /cpp/ncnn-android)
set(NCNN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../cpp/ncnn-android)

# 3. Include directories so the compiler finds the header files
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../cpp
    ${REACT_NATIVE_DIR}/React
    ${REACT_NATIVE_DIR}/React/Base
    ${REACT_NATIVE_DIR}/ReactCommon/jsi
    ${NCNN_DIR}/include/ncnn
)

# 4. Add your C++ source files
add_library(
    ghostsync
    SHARED
    ../cpp/engine.cpp
    ../cpp/model_runner.cpp
    ../cpp/jsi_bindings.cpp
)

# 5. Import the pre-compiled NCNN static library for Android
add_library(ncnn STATIC IMPORTED)
set_target_properties(ncnn PROPERTIES IMPORTED_LOCATION ${NCNN_DIR}/${ANDROID_ABI}/libncnn.a)

# 6. Find Android's built-in logging library (for debugging)
find_library(log-lib log)

# 7. Link everything together!
target_link_libraries(
    ghostsync
    ncnn
    ${log-lib}
)