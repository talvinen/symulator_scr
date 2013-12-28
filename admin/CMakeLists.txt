CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
 
PROJECT(simulator_scr C)

set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

# create a list of source files (easier to maintain)
set(sources ../src/simulator_scr.c ../src/simulator_scr.h ../src/simulator_config.c ../src/simulator_config.h)
 
ADD_EXECUTABLE(simulator_scr ${sources})
 
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(GTK REQUIRED gtk+-2.0)
PKG_SEARCH_MODULE(GTHREAD REQUIRED gthread-2.0)
INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})

FIND_PACKAGE(Threads)

TARGET_LINK_LIBRARIES(simulator_scr ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} cairo config)
