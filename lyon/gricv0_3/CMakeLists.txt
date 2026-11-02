cmake_minimum_required(VERSION 3.6)

project(lyon_gricv0_3 VERSION 0.1.0 DESCRIPTION "GRICV0 using shared libhv transport")

set(SRCS
  board.cc
  datahandler.cc
  interface.cc
  registerhandler.cc
  sensorhandler.cc
  Gricv0_3Manager.cc
)

set(HEADERS
  Gricv0_3Interface.hh
  Gricv0_3Manager.hh
)

add_library(lyon_gricv0_3 SHARED ${SRCS} ${HEADERS})

target_link_libraries(lyon_gricv0_3 PRIVATE pmdaq lyon_code_libhv lyon_db evb zmq ${PMDAQ_LIBS})
target_compile_features(lyon_gricv0_3 PRIVATE cxx_std_11)

include(../../CMake.defs)

target_include_directories(lyon_gricv0_3 PRIVATE
  ../../include
  ../db
  ../../evb/include
)

install(TARGETS lyon_gricv0_3 LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
