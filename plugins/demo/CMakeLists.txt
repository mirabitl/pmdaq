 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_demo VERSION 0.0.1 DESCRIPTION "Demo plugin")



set(SRCS
  demo.cxx
	)
	
set(HEADERS
	demo.hh
	)

add_library(app_demo SHARED ${SRCS} ${HEADERS})

include(../../CMake.defs)


#target_link_libraries(app_demo PRIVATE cpprest)

#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(app_demo PRIVATE pmdaq ${PMDAQ_LIBS} )

target_include_directories(app_demo PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_demo  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


