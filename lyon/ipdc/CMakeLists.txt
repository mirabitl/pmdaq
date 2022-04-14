 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_ipdc VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  FtdiUsbDriver.cc
  IpdcHandler.cc
  IpdcManager.cc

  )
	
set(HEADERS
  FtdiUsbDriver.hh
  IpdcHandler.hh
  IpdcManager.hh
  )
link_directories( ../../lib )
add_library(lyon_ipdc SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_ipdc PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_ipdc PRIVATE pmdaq ${PMDAQ_LIBS})
#target_include_directories(lyon_ipdc PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_ipdc PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_ipdc PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_ipdc PRIVATE log4cxx)
if (APPLE)
target_link_libraries(lyon_ipdc PRIVATE  ftdi1 pthread)
target_include_directories(lyon_ipdc PRIVATE /usr/local/include/libftdi1)
else()
target_link_libraries(lyon_ipdc PRIVATE ftdi pthread)
endif()
target_include_directories(lyon_ipdc PRIVATE ../../include   )
target_include_directories(lyon_ipdc PRIVATE /usr/include/libftdi1)


# Créé des variables avec les fichiers à coipdcler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_ipdc  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


