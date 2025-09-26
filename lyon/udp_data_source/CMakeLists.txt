 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_udp_data_source VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  udp_data_source.cc
  )
	
set(HEADERS
  udp_data_source.hh
  )

link_directories( ../../lib )
add_library(lyon_udp_data_source SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_udp_data_source PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_udp_data_source PRIVATE pmdaq zmq ${PMDAQ_LIBS})
#target_include_directories(lyon_udp_data_source PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_udp_data_source PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_udp_data_source PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_udp_data_source PRIVATE log4cxx)
target_link_libraries(lyon_udp_data_source PRIVATE  evb)

target_include_directories(lyon_udp_data_source PRIVATE ../../include ../../evb/include )


# Créé des variables avec les fichiers à coudp_data_sourceler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_udp_data_source  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


