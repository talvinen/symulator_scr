CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
 
PROJECT(simulator_scr C)

set(Tutorial_VERSION_MAJOR 1)
set(Tutorial_VERSION_MINOR 0)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -Werror -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O3")

set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/../src/)
set(simulator_src_dir ${src_dir}/simulator/)
set(driver_src_dir ${src_dir}/driver/)

# create a list of source files (easier to maintain)
set(sources_simulator ${simulator_src_dir}/simulator_scr.c
					  ${simulator_src_dir}/simulator_scr.h
					  ${simulator_src_dir}/simulator_config.c
					  ${simulator_src_dir}/simulator_config.h
					  ${src_dir}/common/rpc_all.h)
set(sources_driver ${driver_src_dir}/client_scr.c)
 
ADD_EXECUTABLE(simulator_scr ${sources_simulator})
ADD_EXECUTABLE(driver_scr ${sources_driver})
 
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(GTK REQUIRED gtk+-2.0)
PKG_SEARCH_MODULE(GTHREAD REQUIRED gthread-2.0)
INCLUDE_DIRECTORIES(${GTK_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${GTHREAD_INCLUDE_DIRS})

FIND_PACKAGE(Threads)

TARGET_LINK_LIBRARIES(simulator_scr ${GTK_LIBRARIES} ${GTHREAD_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} cairo config tpl)
