 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_db VERSION 0.0.1 DESCRIPTION "Event builder plugin")

#add_subdirectory(pns)

set(SRCS
  HR2ConfigAccess.cc Febv1ConfigAccess.cc
  )
	
set(HEADERS

  HR2Slow.hh HR2ConfigAccess.hh PRSlow.hh PRBSlow.hh Febv1ConfigAccess.hh
  )
      
add_library(lyon_db SHARED ${SRCS} ${HEADERS})


include(../../CMake.defs)


target_link_libraries(lyon_db PRIVATE  zmq pmdaq evb ${PMDAQ_LIBS} )



target_include_directories(lyon_db PRIVATE  ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_db  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

