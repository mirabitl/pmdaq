 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_dif VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  ../mdcc/FtdiUsbDriver.cc
  driver.cc
  reader.cc
  interface.cc
  DifManager.cc
  )
	
set(HEADERS
  ../mdcc/FtdiUsbDriver.hh
  driver.hh
  reader.hh
  interface.hh
  DifManager.hh
  )
link_directories( ../../lib )
add_library(lyon_dif SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_mdcc PUBLIC ${ZUP_LIB})
if (APPLE)
target_link_libraries(lyon_dif PRIVATE  pmdaq zmq evb lyon_db ftdi1 pthread ${PMDAQ_LIBS})
target_include_directories(lyon_dif PRIVATE /usr/local/include/libftdi1)
else()
  target_link_libraries(lyon_dif PRIVATE pmdaq zmq evb lyon_db ftdi pthread ${PMDAQ_LIBS})
  target_include_directories(lyon_dif PRIVATE /usr/include/libftdi1)
endif()
#target_include_directories(lyon_mdcc PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_mdcc PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_mdcc PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_mdcc PRIVATE log4cxx)

target_include_directories(lyon_dif PRIVATE ../db ../mdcc ../../include ../../evb/include   )


# Créé des variables avec les fichiers à comdccler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_dif  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


