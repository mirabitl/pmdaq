 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(pns VERSION 0.0.1 DESCRIPTION "Process name server library")

#add_subdirectory(pns)

set(SRCS
  pns.cxx
	)
	
set(HEADERS
	pns.hh
	)

add_library(pns SHARED ${SRCS} ${HEADERS})




ADD_DEFINITIONS(-std=c++11)

#SET(cpprestsdk_DIR /usr/lib/x86_64-linux-gnu/cmake)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  SET(cpprestsdk_DIR /usr/lib/x86_64-linux-gnu/cmake)
endif()
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
  SET(cpprestsdk_DIR /usr/lib/arm-linux-gnueabihf/cmake)
endif()

find_package(cpprestsdk CONFIG REQUIRED)
target_link_libraries(pns PRIVATE cpprest)

#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(pns PRIVATE log4cxx)

target_include_directories(pns PRIVATE ../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS pns  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


add_executable(pnse pns.cc ${HEADERS})



set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(pnse PRIVATE Threads::Threads)

target_include_directories(pnse PRIVATE ../include)
target_link_libraries(pnse PRIVATE pmdaq cpprest log4cxx boost_system crypto ssl )




install(TARGETS pnse  DESTINATION bin  ARCHIVE DESTINATION bin)
