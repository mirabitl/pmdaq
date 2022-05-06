 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(mqttStore VERSION 0.0.1 DESCRIPTION "MqttStore plugin")



set(SRCS
  mqttStore.cxx 
	)
	
set(HEADERS
	mqttStore.hh  
	)

add_library(mqttStore SHARED ${SRCS} ${HEADERS})


include(../../CMake.defs)

target_link_libraries(mqttStore PRIVATE mosquitto pmdaq ${PMDAQ_LIBS})


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(mqttStore PRIVATE log4cxx)


target_include_directories(mqttStore PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS mqttStore  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


