 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_wtricv1 VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  datahandler.cc
  registerhandler.cc
  slchandler.cc
  Wtricv1Manager.cc
  )
	
set(HEADERS
  Wtricv1Interface.hh
  Wtricv1Manager.hh
  )
link_directories( ../../lib )
add_library(lyon_wtricv1 SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_wtricv1 PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_wtricv1 PRIVATE pmdaq zmq ${PMDAQ_LIBS})
#target_include_directories(lyon_wtricv1 PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_wtricv1 PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_wtricv1 PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_wtricv1 PRIVATE log4cxx)
target_link_libraries(lyon_wtricv1 PRIVATE netLink lyon_wizcc lyon_db evb)

target_include_directories(lyon_wtricv1 PRIVATE ../../include ../wizcc ../db ../../evb/include )


# Créé des variables avec les fichiers à cowtricv1ler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_wtricv1  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


