 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(grStore VERSION 0.0.1 DESCRIPTION "GrStore plugin")



set(SRCS
  grStore.cxx graphite-client.c
	)
	
set(HEADERS
	grStore.hh  graphite-client.h
	)

add_library(grStore SHARED ${SRCS} ${HEADERS})



ADD_DEFINITIONS(-std=c++11)

#SET(cpprestsdk_DIR /usr/lib/x86_64-linux-gnu/cmake)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  SET(cpprestsdk_DIR /usr/lib/x86_64-linux-gnu/cmake)
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
  SET(cpprestsdk_DIR /usr/lib/arm-linux-gnueabihf/cmake)
endif()

find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(grStore PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(grStore PRIVATE log4cxx)


target_include_directories(grStore PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS grStore  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


