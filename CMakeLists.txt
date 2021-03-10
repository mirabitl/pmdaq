 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(pmdaq VERSION 0.0.1 DESCRIPTION "PM daq base library")
message(${CMAKE_SYSTEM_PROCESSOR})
add_subdirectory(pns)
add_subdirectory(plugins/demo)
add_subdirectory(plugins/zup)
add_subdirectory(plugins/wiener)
add_subdirectory(plugins/grStore)
add_subdirectory(plugins/mgStore)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  add_subdirectory(plugins/bmp)
endif()
set(SRCS
	src/baseServer.cxx src/fsmw.cxx src/monitor.cxx
	)
	
set(HEADERS
	include/baseServer.hh include/fsmw.hh include/handlerPlugin.hh include/monitor.hh include/pmLogger.hh include/stdafx.hh
	)

add_library(pmdaq SHARED ${SRCS} ${HEADERS})


include(CMake.defs)

target_link_libraries(pmdaq PRIVATE cpprest)

#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(pmdaq PRIVATE log4cxx)

target_include_directories(pmdaq PRIVATE include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS pmdaq  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


add_executable(pmd src/baseServer.cc ${HEADERS})



target_link_libraries(pmd PRIVATE Threads::Threads)

target_include_directories(pmd PRIVATE include)
target_link_libraries(pmd PRIVATE pmdaq cpprest log4cxx boost_system crypto ssl )




install(TARGETS pmd  DESTINATION bin  ARCHIVE DESTINATION bin)
