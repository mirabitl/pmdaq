 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_hih VERSION 0.0.1 DESCRIPTION "Hih plugin")



set(SRCS
  hihPlugin.cxx
	)
	
set(HEADERS
  hihPlugin.hh
  hih8000.hh
  )


include(../../CMake.defs)

add_library(app_hih SHARED ${SRCS} ${HEADERS})

target_link_libraries(app_hih PUBLIC ${PMDAQLIB})
target_include_directories(app_hih PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_hih  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


