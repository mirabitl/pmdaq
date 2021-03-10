 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_wiener VERSION 0.0.1 DESCRIPTION "Wiener plugin")



set(SRCS
  wienerPlugin.cxx
	)
	
set(HEADERS
	wienerPlugin.hh
	)

add_library(app_wiener SHARED ${SRCS} ${HEADERS})

set(wiener_DIR /opt/lydaq/drivers/Wiener )
find_library(WIENER_LIB lydaq_Wiener PATHS /opt/lydaq/drivers/Wiener/lib)
if(NOT WIENER_LIB)
  message(FATAL_ERROR "WIENER_LIB library not found")
endif()



target_link_libraries(app_wiener PUBLIC ${WIENER_LIB})
target_include_directories(app_wiener PRIVATE ${wiener_DIR}/include)

target_include_directories(app_wiener PRIVATE /usr/include/jsoncpp)
ADD_DEFINITIONS(-std=c++11)

SET(cpprestsdk_DIR /usr/lib/x86_64-linux-gnu/cmake)

find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(app_wiener PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(app_wiener PRIVATE log4cxx)
target_link_libraries(app_wiener PRIVATE jsoncpp)

target_include_directories(app_wiener PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_wiener  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


