 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_gricv1_3 VERSION 0.1.0 DESCRIPTION "Gricv1 using libhv")



set(SRCS
  board.cc
  datahandler.cc
  interface.cc
  registerhandler.cc
  sockethandler.cc
  slchandler.cc
  Gricv1_3Manager.cc
  )
	
set(HEADERS
  Gricv1Interface.hh
  Gricv1_3Manager.hh
  )
link_directories( ../../lib )
add_library(lyon_gricv1_3 SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_gricv1_3 PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_gricv1_3 PRIVATE pmdaq zmq ${PMDAQ_LIBS})
#target_include_directories(lyon_gricv1_3 PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_gricv1_3 PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_gricv1_3 PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_gricv1_3 PRIVATE log4cxx)
target_link_libraries(lyon_gricv1_3 PRIVATE lyon_mpi2 lyon_db evb)

target_include_directories(lyon_gricv1_3 PRIVATE ../../include ../mpi2 ../db ../../evb/include )


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_gricv1_3  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
