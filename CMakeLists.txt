 #Nous voulons un cmake "récent" pour utiliser les dernières fonctionnalités
cmake_minimum_required(VERSION 3.0)

# Notre projet est étiqueté libhello

project(pmdaq VERSION 0.1.0 DESCRIPTION "PM daq base library")
if(UNIX AND NOT APPLE)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Laurent Mirabito")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libzmq3-dev,libcpprest-dev,libftdi-dev,liblog4cxx-dev,libboost-system-dev,libmongoc-dev")
endif()
set(CPACK_PACKAGE_NAME "pmdaq")
set(CPACK_PACKAGE_VENDOR "ip2i")
set(CPACK_VERBATIM_VARIABLES true)
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX /usr/local/pmdaq)
set(FAKE_TARGET fakeTarget)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/netlink/config.h )
  add_custom_target(fakeTarget  echo "netLink already installed")
else()
  if (NOT APPLE AND UNIX)
    add_custom_target(fakeTarget  ${CMAKE_CURRENT_SOURCE_DIR}/extras/install_netlink)
  else()
    add_custom_target(fakeTarget  echo "netLink not yet installed")
  endif()
endif()

include(CMake.defs)

message(${CMAKE_SYSTEM_PROCESSOR})
add_subdirectory(pns)
add_subdirectory(evb)
add_subdirectory(plugins/demo)
if (NOT APPLE)
  add_subdirectory(plugins/zup)
  add_subdirectory(plugins/genesys)
  add_subdirectory(plugins/wiener)
endif()
add_subdirectory(plugins/grStore)
add_subdirectory(plugins/mgStore)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "^arm")
  set(CPACK_SYSTEM_NAME "Linux-arm")
  set(CPACK_TOPLEVEL_TAG "Linux-arm")
  add_subdirectory(plugins/bmp)
  add_subdirectory(plugins/hih)
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64")
  set(CPACK_SYSTEM_NAME "Linux-x86_64")
  set(CPACK_TOPLEVEL_TAG "Linux-x86_64")
  if (NOT APPLE)
    add_subdirectory(plugins/syx27)
  endif()
endif()
add_subdirectory(lyon/db)
add_subdirectory(lyon/mpi)
add_subdirectory(lyon/febv1)
add_subdirectory(lyon/mbmdcc)
add_subdirectory(lyon/gricv1)
add_subdirectory(lyon/pmr)
add_subdirectory(lyon/mdcc)

include(CPack)


set(SRCS
	src/baseServer.cxx src/fsmw.cxx src/monitor.cxx src/utils.cxx
	)
	
set(HEADERS
	include/baseServer.hh include/fsmw.hh include/handlerPlugin.hh include/monitor.hh include/pmLogger.hh include/stdafx.hh include/utils.hh
	)

add_library(pmdaq SHARED ${SRCS} ${HEADERS})

add_dependencies(pmdaq ${FAKE_TARGET})






target_link_libraries(pmdaq PRIVATE ${PMDAQ_LIBS})

#SET(log4cxx_DIR /usr/lib/x86_64-linux-gnu/cmake)
#find_package(log4cxx CONFIG REQUIRED)
target_link_libraries(pmdaq PRIVATE log4cxx)

target_include_directories(pmdaq PRIVATE include)


# Créé des variables avec les fichiers à compiler

# La bibliothèque peut être soit une bibliothèque statique, soit dynamique, donc on précise où installer pour les deux cas
install(TARGETS pmdaq  LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)


add_executable(pmd src/baseServer.cc ${HEADERS})



target_link_libraries(pmd PRIVATE Threads::Threads)

target_include_directories(pmd PRIVATE include)
target_link_libraries(pmd PRIVATE pmdaq ${PMDAQ_LIBS})


set(SCRIPT_TARGET ${CMAKE_CURRENT_SOURCE_DIR}/scripts/session.py ${CMAKE_CURRENT_SOURCE_DIR}/scripts/serviceAccess.py     )
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/scripts )
  install(FILES ${SCRIPT_TARGET}  DESTINATION share )
  install(FILES etc/pmdaq.conf etc/pns.conf DESTINATION etc)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/bin/pmdaemon ${CMAKE_CURRENT_SOURCE_DIR}/bin/pnsdaemon DESTINATION bin)
endif()
install(TARGETS pmd  DESTINATION bin  ARCHIVE DESTINATION bin)
