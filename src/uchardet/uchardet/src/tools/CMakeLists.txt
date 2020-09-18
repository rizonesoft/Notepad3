set(
	UCHARDET_SOURCES
	uchardet.cpp
)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(
		-O0
		-g3
		-fsanitize=address
	)
	set(CMAKE_CXX_FLAGS "-fsanitize=address")
	set(CMAKE_C_FLAGS "-fsanitize=address")
	set(CMAKE_EXE_LINKER_FLAGS "-fsanitize=address")
	set(CMAKE_MODULE_LINKER_FLAGS "-fsanitize=address")
	set(CMAKE_SHARED_LINKER_FLAGS "-fsanitize=address")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

set(UCHARDET_BINARY uchardet)

add_executable(
	${UCHARDET_BINARY}
	${UCHARDET_SOURCES}
)

target_link_libraries(
	${UCHARDET_BINARY}
	${UCHARDET_LIBRARY}
)

install(
	TARGETS
		${UCHARDET_BINARY}
	RUNTIME DESTINATION
		${CMAKE_INSTALL_BINDIR}
)
