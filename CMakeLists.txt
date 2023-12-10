cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(qrar VERSION 0.1.0)

# include(CTest)
# enable_testing()

find_package( OpenCV REQUIRED )

include_directories( ${OpenCV_INCLUDE_DIRS} "zxing-cpp/core/src" "zxing-cpp/example" "json/include" "json/single_include" "jansson/build/include" )

link_directories("zxing-cpp/build/core/Release")

find_library(ZXing
             NAMES ZXing.lib
             PATHS "zxing-cpp/build/core/Release")

add_subdirectory(json)

add_subdirectory(jansson)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

add_subdirectory(OpenXLSX)

add_executable(qrar main.cpp utils.cpp)

target_link_libraries( qrar ${OpenCV_LIBS} ZXing OpenXLSX::OpenXLSX nlohmann_json::nlohmann_json)

add_executable(students-data students-data.c students-data-utils.c)

add_executable(qr-code-generator qr-code-generator.cpp utils.cpp)

target_link_libraries( students-data jansson)

target_link_libraries( qr-code-generator ${OpenCV_LIBS} ZXing OpenXLSX::OpenXLSX nlohmann_json::nlohmann_json)

if (MSVC)
    target_compile_options(qrar PRIVATE /W3)
    target_compile_options(students-data PRIVATE /W3)
    target_compile_options(qr-code-generator PRIVATE /W3)
endif()

if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(qrar PRIVATE -Wall -Wextra -Werror)
    target_compile_options(students-data PRIVATE -Wall -Wextra -Werror)
    target_compile_options(qr-code-generator PRIVATE -Wall -Wextra -Werror)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)