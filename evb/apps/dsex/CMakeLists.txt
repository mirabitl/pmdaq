 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(evb_producer VERSION 0.0.1 DESCRIPTION "Event producer plugin")

#add_subdirectory(pns)

set(SRCS
  localProducer.cc
  )
	
set(HEADERS

  localProducer.hh
  )
      
add_library(evb_producer SHARED ${SRCS} ${HEADERS})


include(../../../CMake.defs)


target_link_libraries(evb_producer PRIVATE  zmq evb ${PMDAQ_LIBS} )



target_include_directories(evb_producer PRIVATE  ../../include ../../../include )


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS evb_producer  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

