cmake_minimum_required(VERSION 3.6)

project(lyon_mpi2 VERSION 0.1.0 DESCRIPTION "libhv-based MPI transport")

find_package(libhv CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_library(lyon_mpi2 SHARED
  MpiMessageHandler.cc
  MessageHandler.hh
  MpiMessageHandler.hh
)

target_compile_features(lyon_mpi2 PUBLIC cxx_std_11)
target_include_directories(lyon_mpi2
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE
    ../../include
)
target_link_libraries(lyon_mpi2
  PUBLIC
    hv
  PRIVATE
    Threads::Threads
)

install(TARGETS lyon_mpi2 LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
