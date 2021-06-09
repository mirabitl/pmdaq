 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_liboard VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  LiboardDriver.cc
  LiboardInterface.cc
  LiboardManager.cc
  )
	
set(HEADERS
  LiboardDriver.hh
  LiboardInterface.hh
  LiboardManager.hh
  )
link_directories( ../../lib )
add_library(lyon_liboard SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_liboard PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_liboard PRIVATE zmq pmdaq ${PMDAQ_LIBS})
#target_include_directories(lyon_liboard PRIVATE ${zup_DIR}/include)

target_include_directories(lyon_liboard PRIVATE /usr/include/libftdi1)


include(../../CMake.defs)

#target_link_libraries(lyon_liboard PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_liboard PRIVATE log4cxx)
if (NOT APPLE)
target_link_libraries(lyon_liboard PRIVATE netLink ftdi lyon_db evb)
else()
target_link_libraries(lyon_liboard PRIVATE netLink ftdi1 lyon_db evb)
target_include_directories(lyon_liboard PRIVATE /usr/local/include/libftdi1)
endif()

target_include_directories(lyon_liboard PRIVATE ../../include  ../db ../../evb/include )


# Créé des variables avec les fichiers à coliboardler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_liboard  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


