 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_febinj VERSION 0.0.1 DESCRIPTION "Febinj plugin")



set(SRCS
  FebinjManager.cc
	)
	
set(HEADERS
  FebinjManager.hh
  febinj.hh
  )


include(../../CMake.defs)

add_library(lyon_febinj SHARED ${SRCS} ${HEADERS})

target_link_libraries(lyon_febinj PUBLIC pmdaq ${PMDAQLIB})
target_include_directories(lyon_febinj PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_febinj  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


