cmake_minimum_required(VERSION 3.16)

project(rbana LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)

pkg_check_modules(JSONCPP REQUIRED jsoncpp)
pkg_check_modules(ZSTD REQUIRED libzstd)

find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})
include_directories(
    src
    ${JSONCPP_INCLUDE_DIRS}
    ${ZSTD_INCLUDE_DIRS}
)

add_library(rbcore SHARED
    src/rbReader.cc
    src/DCHistogramHandler.cc
    src/TdcUplinkFrame.cc
    src/irpcGeometry.cc
)

target_link_libraries(rbcore
    ${JSONCPP_LIBRARIES}
    ${ZSTD_LIBRARIES}
    ${ROOT_LIBRARIES}
    ROOT::XMLIO
)

add_executable(rbana
    src/main.cc
)

target_link_libraries(rbana
    rbcore
    dl
     ${ROOT_LIBRARIES}
)

add_library(ExampleProcessor SHARED
    plugins/ExampleProcessor.cc
)

target_link_libraries(
    ExampleProcessor
    rbcore
    ${JSONCPP_LIBRARIES}
)
add_library(picProcessor SHARED
    plugins/picProcessor.cc
)

target_include_directories(picProcessor PRIVATE
    src
    ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(picProcessor
    rbcore
    ${ROOT_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)

add_library(FebProcessor SHARED
    plugins/FebProcessor.cc
)

target_include_directories(FebProcessor PRIVATE
    src
    ${ROOT_INCLUDE_DIRS}
)

target_link_libraries(FebProcessor
    rbcore
    ${ROOT_LIBRARIES}
    ${JSONCPP_LIBRARIES}
)
