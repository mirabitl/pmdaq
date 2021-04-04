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


include(../CMake.defs)


target_link_libraries(pns PRIVATE pmdaq ${PMDAQ_LIBS})


#target_link_libraries(pns PRIVATE log4cxx)

target_include_directories(pns PRIVATE ../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS pns  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


add_executable(pnse pns.cc ${HEADERS})



#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
target_link_libraries(pnse PRIVATE Threads::Threads pmdaq ${PMDAQ_LIBS})

target_include_directories(pnse PRIVATE ../include)
#target_link_libraries(pnse PRIVATE pmdaq cpprest log4cxx boost_system crypto ssl )




install(TARGETS pnse  DESTINATION bin  ARCHIVE DESTINATION bin)
