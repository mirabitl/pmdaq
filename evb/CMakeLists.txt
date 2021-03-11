 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(evb VERSION 0.0.1 DESCRIPTION "Event builder library")

#add_subdirectory(pns)
add_subdirectory(apps/builder)
set(SRCS
  src/pmSender.cc
  src/pmPuller.cc
  src/pmMerger.cc
	)
	
set(HEADERS
  include/pmBuffer.hh
  include/pmMerger.hh
  include/pmPuller.hh
  include/pmSender.hh
  include/zhelpers.hpp

	)

add_library(evb SHARED ${SRCS} ${HEADERS})


include(../CMake.defs)


target_link_libraries(evb PRIVATE ${PMDAQ_LIBS} zmq )



target_include_directories(evb PRIVATE include ../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS evb  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

