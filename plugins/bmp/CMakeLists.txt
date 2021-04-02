 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(app_bmp VERSION 0.0.1 DESCRIPTION "Bmp plugin")



set(SRCS
  bmpPlugin.cxx
  )
	
set(HEADERS
  bmpPlugin.hh
  bmp280.hh
  )

add_library(app_bmp SHARED ${SRCS} ${HEADERS})

#set(bmp_DIR /opt/lydaq/drivers/Bmp )
#find_library(BMP_LIB lydaq_drivers PATHS /usr/local/lydaq/lib/)
#if(NOT BMP_LIB)
#  message(FATAL_ERROR "BMP_LIB library not found")
#endif()



#target_link_libraries(app_bmp PUBLIC ${BMP_LIB})
#target_include_directories(app_bmp PRIVATE ${bmp_DIR}/include)

#target_include_directories(app_bmp PRIVATE /usr/include/jsoncpp)


include(../../CMake.defs)

target_include_directories(app_bmp PRIVATE ../../include)
target_link_libraries(app_bmp PRIVATE ${PMDAQLIBS})


#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
#target_link_libraries(app_bmp PRIVATE log4cxx)
#target_link_libraries(app_bmp PRIVATE jsoncpp)




# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS app_bmp  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


