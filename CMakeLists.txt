cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qrar VERSION 0.1.0)

# include(CTest)
# enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} "zxing-cpp/core/src" "zxing-cpp/example" "json/include" "json/single_include" )
link_directories("zxing-cpp/build/core/Release")

find_library(ZXing
             NAMES ZXing.lib
             PATHS "zxing-cpp/build/core/Release")

add_subdirectory(json)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

add_subdirectory(OpenXLSX)

add_executable(qrar main.cpp utils.cpp)

target_link_libraries( qrar ${OpenCV_LIBS} ZXing OpenXLSX::OpenXLSX nlohmann_json::nlohmann_json)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)