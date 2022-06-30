 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_dbtool VERSION 0.0.1 DESCRIPTION "Event builder plugin")

#add_subdirectory(pns)

set(SRCS
  configaccess.cc 
  )
add_executable(dbt ${SRCS})
	
target_link_libraries(dbt PRIVATE  jsoncpp )



target_include_directories(dbt PRIVATE  /usr/include/jsoncpp)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas

install(TARGETS dbt DESTINATION bin  ARCHIVE DESTINATION bin)
