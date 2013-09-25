
cmake_minimum_required(VERSION 2.8)

project(ctcompare C)

set(CMAKE_COLOR_MAKEFILE ON)

# libctf

set(MODULE_NAME "ctf")
set(MODULE_PREFIX "CTF")

set(${MODULE_PREFIX}_SRCS
	lctf.h
	libtdn.c
	liblexer.c
	libbuildctf.c
	libtokens.c
	libruns.c
	libprintruns.c
	libctflist.c)

add_library(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

# ctcompare

set(MODULE_NAME "ctcompare")
set(MODULE_PREFIX "CTCOMPARE")

set(${MODULE_PREFIX}_SRCS
	ctcompare.c)

add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

set(${MODULE_PREFIX}_LIBS ctf)

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})

# twoctcompare

set(MODULE_NAME "twoctcompare")
set(MODULE_PREFIX "TWOCTCOMPARE")

set(${MODULE_PREFIX}_SRCS
	twoctcompare.c)

add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

set(${MODULE_PREFIX}_LIBS ctf)

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})

# detok

set(MODULE_NAME "detok")
set(MODULE_PREFIX "DETOK")

set(${MODULE_PREFIX}_SRCS
	detok.c)

add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})

set(${MODULE_PREFIX}_LIBS ctf)

target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})
