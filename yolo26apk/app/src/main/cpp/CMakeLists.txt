cmake_minimum_required(VERSION 3.22.1)
project(yolo26_ncnn)

add_library(yolo26_ncnn SHARED yolo26_ncnn_jni.cpp)

find_library(log-lib log)
find_library(jnigraphics-lib jnigraphics)

if (NOT NCNN_DIR)
    message(FATAL_ERROR "NCNN_DIR is empty. Please set ncnn.dir in local.properties")
endif ()

if (EXISTS "${NCNN_DIR}/ncnnConfig.cmake")
    include("${NCNN_DIR}/ncnnConfig.cmake")
else ()
    message(FATAL_ERROR "ncnnConfig.cmake not found in NCNN_DIR=${NCNN_DIR}")
endif ()

target_link_libraries(yolo26_ncnn ncnn ${log-lib} ${jnigraphics-lib})
