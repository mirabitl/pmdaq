 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_ptdc VERSION 0.0.1 DESCRIPTION "PTDC plugin")



set(SRCS
  PTDCManager.cc
  )
	
set(HEADERS
  PTDCManager.hh
  )

link_directories( ../../lib )
add_library(lyon_ptdc SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_ptdc PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_ptdc PRIVATE pmdaq zmq ${PMDAQ_LIBS})
#target_include_directories(lyon_ptdc PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_ptdc PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_ptdc PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_ptdc PRIVATE log4cxx)
target_link_libraries(lyon_ptdc PRIVATE  evb)

target_include_directories(lyon_ptdc PRIVATE ../../include ../../evb/include )


# Créé des variables avec les fichiers à coptdcler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_ptdc  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


