cmake_minimum_required(VERSION 3.6)

project(lyon_mbmdcc_2 VERSION 0.1.0 DESCRIPTION "MBMDCC using shared libhv transport")

set(SRCS
  board.cc
  interface.cc
  registerhandler.cc
  MBMDCCManager.cc
)

set(HEADERS
  MBMDCCInterface.hh
  MBMDCCManager.hh
)

add_library(lyon_mbmdcc_2 SHARED ${SRCS} ${HEADERS})

target_link_libraries(lyon_mbmdcc_2 PRIVATE pmdaq lyon_code_libhv evb ${PMDAQ_LIBS})
target_compile_features(lyon_mbmdcc_2 PRIVATE cxx_std_11)

include(../../CMake.defs)

target_include_directories(lyon_mbmdcc_2 PRIVATE
  ../../include
  ../../evb/include
)

install(TARGETS lyon_mbmdcc_2 LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
