 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_genesys VERSION 0.0.1 DESCRIPTION "Genesys plugin")



set(SRCS
  genesysPlugin.cxx
	)
	
set(HEADERS
	genesysPlugin.hh
	)

add_library(app_genesys SHARED ${SRCS} ${HEADERS})

set(genesys_DIR /opt/lydaq/drivers/Genesys )
find_library(GENESYS_LIB lydaq_Genesys PATHS /opt/lydaq/drivers/Genesys/lib)
if(NOT GENESYS_LIB)
  message(FATAL_ERROR "GENESYS_LIB library not found")
endif()



target_link_libraries(app_genesys PUBLIC ${GENESYS_LIB})
target_include_directories(app_genesys PRIVATE ${genesys_DIR}/include)

target_include_directories(app_genesys PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

target_link_libraries(app_genesys PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(app_genesys PRIVATE log4cxx)
target_link_libraries(app_genesys PRIVATE jsoncpp)

target_include_directories(app_genesys PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_genesys  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


