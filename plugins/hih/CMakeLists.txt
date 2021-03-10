 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_hih VERSION 0.0.1 DESCRIPTION "Hih plugin")



set(SRCS
  hihPlugin.cxx
	)
	
set(HEADERS
	hihPlugin.hh
	)

add_library(app_hih SHARED ${SRCS} ${HEADERS})

set(hih_DIR /opt/lydaq/drivers/hih8000 )
find_library(HIH_LIB lydaq_hih8000 PATHS ${hih_DIR}/lib)
if(NOT HIH_LIB)
  message(FATAL_ERROR "HIH_LIB library not found")
endif()



target_link_libraries(app_hih PUBLIC ${HIH_LIB})
target_include_directories(app_hih PRIVATE ${hih_DIR}/include)

target_include_directories(app_hih PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

target_link_libraries(app_hih PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(app_hih PRIVATE log4cxx)
target_link_libraries(app_hih PRIVATE jsoncpp)

target_include_directories(app_hih PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_hih  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


