 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(lyon_sdcc VERSION 0.0.1 DESCRIPTION "Zup plugin")



set(SRCS
  ../mdcc/FtdiUsbDriver.cc
  driver.cc
  reader.cc
  interface.cc
  SdccManager.cc
  )
	
set(HEADERS
  ../mdcc/FtdiUsbDriver.hh
  driver.hh
  reader.hh
  interface.hh
  SdccManager.hh

  )
link_directories( ../../lib )
add_library(lyon_sdcc SHARED ${SRCS} ${HEADERS})

#set(zup_DIR /opt/lydaq/drivers/Zup )
#find_library(ZUP_LIB lydaq_Zup PATHS /opt/lydaq/drivers/Zup/lib)
#if(NOT ZUP_LIB)
 # message(FATAL_ERROR "ZUP_LIB library not found")
#endif()



#target_link_libraries(lyon_mdcc PUBLIC ${ZUP_LIB})
target_link_libraries(lyon_sdcc PRIVATE  pmdaq ${PMDAQ_LIBS})
#target_include_directories(lyon_mdcc PRIVATE ${zup_DIR}/include)

#target_include_directories(lyon_mdcc PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

#target_link_libraries(lyon_mdcc PRIVATE cpprest)


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(lyon_mdcc PRIVATE log4cxx)
if (NOT APPLE)
  target_link_libraries(lyon_sdcc PRIVATE ftdi pthread)
  target_include_directories(lyon_sdcc PRIVATE /usr/include/libftdi1)
else()
target_link_libraries(lyon_sdcc PRIVATE  ftdi1 pthread)
target_include_directories(lyon_sdcc PRIVATE /usr/local/include/libftdi1)
endif()
target_include_directories(lyon_sdcc PRIVATE ../mdcc ../../include )


# Créé des variables avec les fichiers à comdccler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS lyon_sdcc  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


