 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(evb_builder VERSION 0.0.1 DESCRIPTION "Event builder plugin")

#add_subdirectory(pns)

set(SRCS
  localCollector.cc
  )
	
set(HEADERS

  localCollector.hh
  )
      
add_library(evb_builder SHARED ${SRCS} ${HEADERS})


include(../../../CMake.defs)


target_link_libraries(evb_builder PRIVATE  paho-mqtt3a paho-mqttpp3  zmq pmdaq evb ${PMDAQ_LIBS} )



target_include_directories(evb_builder PRIVATE  ../../include ../../../include )


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS evb_builder  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

