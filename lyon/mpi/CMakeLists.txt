 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_mpi VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  MessageHandler.cc
  MpiMessageHandler.cc
  )
	
set(HEADERS
  MessageHandler.hh
  MpiMessageHandler.hh
  )

link_directories( ../../lib )
add_library(lyon_mpi SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_mpi PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_mpi PRIVATE netLink pmdaq ${PMDAQ_LIBS})
#target_include_directories(lyon_mpi PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_mpi PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_mpi PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_mpi PRIVATE log4cxx)
target_link_libraries(lyon_mpi PRIVATE netLink)

target_include_directories(lyon_mpi PRIVATE ../../include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_mpi  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


