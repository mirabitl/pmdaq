 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_gricv0 VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  board.cc
  datahandler.cc
  interface.cc
  registerhandler.cc
  sockethandler.cc
  sensorhandler.cc
  messagehandler.cc
  Gricv0Manager.cc
  )
	
set(HEADERS
  Gricv0Interface.hh
  Gricv0Manager.hh
  )
link_directories( ../../lib )
add_library(lyon_gricv0 SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_gricv0 PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_gricv0 PRIVATE ${PMDAQ_LIBS})
#target_include_directories(lyon_gricv0 PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_gricv0 PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_gricv0 PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_gricv0 PRIVATE log4cxx)
target_link_libraries(lyon_gricv0 PRIVATE netLink lyon_mpi lyon_db evb pthread)

target_include_directories(lyon_gricv0 PRIVATE ../../include ../mpi ../db ../../evb/include )


# Créé des variables avec les fichiers à cogricv0ler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_gricv0  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


