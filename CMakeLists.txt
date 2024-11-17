cmake_minimum_required(VERSION 3.10)
project(MNNAgeGoogleNet)
# 使用 Emscripten 工具链
if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    set(CMAKE_CXX_COMPILER em++)
    set(CMAKE_C_COMPILER emcc)
endif()

set(CMAKE_CXX_STANDARD 11)
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
set(THIRD_PARTY_PATH "${CMAKE_SOURCE_DIR}/thirdparty")
include(FetchContent)

function(download_and_decompress url filename dest)
    file(DOWNLOAD ${url} ${CMAKE_BINARY_DIR}/${filename})
    file(MAKE_DIRECTORY ${dest})
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_BINARY_DIR}/${filename} WORKING_DIRECTORY ${dest})
endfunction()

include(cmake/MNN.cmake)
option(ENABLE_MNN "enable MNN engine" ON)

# 打印 ENABLE_MNN 选项的值
message(STATUS "ENABLE_MNN: ${ENABLE_MNN}")

# 设置包含路径
include_directories(${CMAKE_SOURCE_DIR})

# 手动指定 OpenCV 的安装路径
set(OpenCV_DIR "${CMAKE_SOURCE_DIR}/thirdparty/opencv")
set(OpenCV_INCLUDE_DIRS "${OpenCV_DIR}/include/opencv4")
set(OpenCV_LIB_DIR "${OpenCV_DIR}/lib")

include(cmake/opencv.cmake)
# include_directories(${OpenCV_INCLUDE_DIRS})
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV_LIBS: ${OpenCV_LIBS}")

# 打印 MNN_INCLUDE_DIRS 和 MNN_LIBRARIES 变量的值
message(STATUS "MNN_INCLUDE_DIRS: ${MNN_INCLUDE_DIRS}")
message(STATUS "MNN_LIBRARIES: ${MNN_LIBRARIES}")

# 添加源文件
set(SOURCE_FILES ${CMAKE_SOURCE_DIR}/lite/mnn/cv/mnn_age_googlenet.cpp ${CMAKE_SOURCE_DIR}/lite/mnn/core/mnn_handler.cpp)


# add_executable(MNNAgeGoogleNet ${SOURCE_FILES})
# 编译成 WebAssembly
add_executable(mnn_age_googlenet.js ${SOURCE_FILES})

# 链接 MNN 和 OpenCV 库
# target_link_libraries(MNNAgeGoogleNet ${MNN_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(mnn_age_googlenet.js ${MNN_LIBRARIES} ${OpenCV_LIBS} embind)

if("${CMAKE_SYSTEM_NAME}" STREQUAL "Emscripten")
    # 设置编译选项以生成 WASM 模块
    set_target_properties(mnn_age_googlenet.js PROPERTIES
        LINK_FLAGS "-s WASM=1 -s MODULARIZE=1 -s 'EXPORT_NAME=\"MNNAgeGoogleNetModule\"' \
                    -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap','FS_createDataFile'] -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM=1 -s EXPORT_ALL=1 -s NO_DISABLE_EXCEPTION_CATCHING -s ENVIRONMENT=node"
        OUTPUT_NAME "mnn_age_googlenet"
    )
endif()
