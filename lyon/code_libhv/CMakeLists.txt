cmake_minimum_required(VERSION 3.6)

project(lyon_code_libhv VERSION 0.1.0 DESCRIPTION "Shared libhv TCP endpoint")

find_package(libhv CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(lyon_code_libhv SHARED
  SocketHandler.cc
  SocketHandler.hh
  TcpEndpoint.cc
  TcpEndpoint.hh
)
target_compile_features(lyon_code_libhv PUBLIC cxx_std_11)
target_include_directories(lyon_code_libhv PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(lyon_code_libhv PUBLIC hv Threads::Threads)

install(TARGETS lyon_code_libhv LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
