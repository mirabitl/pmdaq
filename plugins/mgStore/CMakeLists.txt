 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(mgStore VERSION 0.0.1 DESCRIPTION "GrStore plugin")



set(SRCS
  mgStore.cxx 
	)
	
set(HEADERS
	mgStore.hh  
	)

add_library(mgStore SHARED ${SRCS} ${HEADERS})

include(../../CMake.defs)


target_link_libraries(mgStore PRIVATE cpprest)
target_link_libraries(mgStore PRIVATE log4cxx)

#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)

# Specific Linkage
target_link_libraries(mgStore PRIVATE mongoc-1.0)
target_link_libraries(mgStore PRIVATE bson-1.0)
target_include_directories(mgStore PRIVATE ../../include)
target_include_directories(mgStore PRIVATE /usr/include/libmongoc-1.0)
target_include_directories(mgStore PRIVATE /usr/include/libbson-1.0)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS mgStore  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


