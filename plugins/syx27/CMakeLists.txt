 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_syx27 VERSION 0.0.1 DESCRIPTION "Syx27 plugin")



set(SRCS
  syx27Plugin.cxx
	)
	
set(HEADERS
	syx27Plugin.hh
	)

add_library(app_syx27 SHARED ${SRCS} ${HEADERS})

#set(syx27_DIR /opt/lydaq/drivers/Caen )
find_library(SYX27_LIB caenhvwrapper  PATHS /usr/lib)

if(NOT SYX27_LIB)
  message(FATAL_ERROR "caenhvwrapper library not found")
endif()



target_link_libraries(app_syx27 PUBLIC ${SYX27_LIB})

#target_include_directories(app_syx27 PRIVATE ${syx27_DIR}/include)

#target_include_directories(app_syx27 PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

target_link_libraries(app_syx27 PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(app_syx27 PRIVATE log4cxx)
#target_link_libraries(app_syx27 PRIVATE jsoncpp)

target_include_directories(app_syx27 PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_syx27  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


