
cmake_minimum_required(VERSION 2.8)

project(ctcompare C)

set(CMAKE_COLOR_MAKEFILE ON)

find_package(FLEX)

# lexers

flex_target(clexer clexer.l ${CMAKE_CURRENT_BINARY_DIR}/clexer.c COMPILE_FLAGS "-Pc_")
flex_target(jlexer jlexer.l ${CMAKE_CURRENT_BINARY_DIR}/jlexer.c COMPILE_FLAGS "-Pj_")
flex_target(pylexer pylexer.l ${CMAKE_CURRENT_BINARY_DIR}/pylexer.c COMPILE_FLAGS "-Ppy_")
flex_target(perllexer perllexer.l ${CMAKE_CURRENT_BINARY_DIR}/perllexer.c COMPILE_FLAGS "-Pperl_")
flex_target(hexlexer hexlexer.l ${CMAKE_CURRENT_BINARY_DIR}/hexlexer.c COMPILE_FLAGS "-Phex_")
flex_target(txtlexer txtlexer.l ${CMAKE_CURRENT_BINARY_DIR}/txtlexer.c COMPILE_FLAGS "-Ptxt_")
flex_target(asmlexer asmlexer.l ${CMAKE_CURRENT_BINARY_DIR}/asmlexer.c COMPILE_FLAGS "-Pasm_")

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

list(APPEND ${MODULE_PREFIX}_SRCS
	${FLEX_clexer_OUTPUTS}
	${FLEX_jlexer_OUTPUTS}
	${FLEX_pylexer_OUTPUTS}
	${FLEX_perllexer_OUTPUTS}
	${FLEX_hexlexer_OUTPUTS}
	${FLEX_txtlexer_OUTPUTS}
	${FLEX_asmlexer_OUTPUTS})

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
