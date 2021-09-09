 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_mbmdcc VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  board.cc
  interface.cc
  registerhandler.cc
  sockethandler.cc
  messagehandler.cc
  MBMDCCManager.cc	
  )
set(HEADERS
  MBMDCCInterface.hh
  MBMDCCManager.hh	
  )

link_directories( ../../lib )
add_library(lyon_mbmdcc SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_mbmdcc PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_mbmdcc PRIVATE pmdaq ${PMDAQ_LIBS})
#target_include_directories(lyon_mbmdcc PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_mbmdcc PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_mbmdcc PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_mbmdcc PRIVATE log4cxx)
target_link_libraries(lyon_mbmdcc PRIVATE netLink  lyon_mpi evb)

target_include_directories(lyon_mbmdcc PRIVATE ../../include ../mpi   ../../evb/include )


# Créé des variables avec les fichiers à combmdccler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_mbmdcc  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


