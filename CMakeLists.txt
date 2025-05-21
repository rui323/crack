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


# 実行ファイルを作成
add_executable(change_file src/change_FileName.cpp)
target_link_libraries(change_file ${dependencies})

add_executable(detect_line src/detect_line.cpp)
target_link_libraries(detect_line ${dependencies})