cmake_minimum_required(VERSION 3.10)
project(cracks)

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# === ONNX Runtime ===
set(ONNXRUNTIME_DIR "onnxruntime-linux-x64-1.21.0")
include_directories(${ONNXRUNTIME_DIR}/include)
link_directories(${ONNXRUNTIME_DIR}/lib)

find_package(OpenCV REQUIRED)

set(HEIF_INCLUDE_DIR /usr/include/libheif)
set(HEIF_LIBRARY heif)

include_directories(
    ${OpenCV_INCLUDE_DIRS}
    ${HEIF_INCLUDE_DIR}
    include
)

set(dependencies
    ${OpenCV_LIBS}
    ${HEIF_LIBRARIES}
)

# *.cpp ファイルを明示的に集める
file(GLOB_RECURSE NN_SOURCES "src/nn/*.cpp")
file(GLOB_RECURSE UTILS_SOURCES "src/utils/*.cpp")

# 実行ファイルを作成
add_executable(change_file src/change_FileName.cpp)
target_link_libraries(change_file ${dependencies})

add_executable(detect_line src/detect_line.cpp)
target_link_libraries(detect_line ${dependencies})

add_executable(find src/find_contours.cpp)
target_link_libraries(find ${dependencies})

# クラック自動検出 main関数から実行
add_executable(cracks_detection 
    src/detection_main.cpp 
    src/detection.cpp 
    ${NN_SOURCES}
    ${UTILS_SOURCES})
target_link_libraries(cracks_detection ${dependencies})
target_link_libraries(cracks_detection onnxruntime) # ONNX

set_target_properties(cracks_detection PROPERTIES 
 INSTALL_RPATH "$(ONNXRUNTIME_DIR)"
 BUILD_RPATH "$(ONNXRUNTIME_DIR)"
)