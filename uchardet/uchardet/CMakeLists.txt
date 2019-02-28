######## Project settings
cmake_minimum_required(VERSION 2.8.5)
include(CheckCCompilerFlag)
set (PACKAGE_NAME uchardet)
project (${PACKAGE_NAME} CXX C)
enable_testing()

######## Package information
set (PACKAGE_URL https://www.freedesktop.org/wiki/Software/uchardet/)
set (PACKAGE_BUGREPORT https://bugs.freedesktop.org/enter_bug.cgi?product=uchardet)
set (UCHARDET_VERSION_MAJOR 0)
set (UCHARDET_VERSION_MINOR 0)
set (UCHARDET_VERSION_REVISION 6)

if (CMAKE_BUILD_TYPE MATCHES Debug)
	set (version_suffix .debug)
endif (CMAKE_BUILD_TYPE MATCHES Debug)

set (
	UCHARDET_VERSION
	${UCHARDET_VERSION_MAJOR}.${UCHARDET_VERSION_MINOR}.${UCHARDET_VERSION_REVISION}${version_suffix}
)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

######## Directory

include(GNUInstallDirs)

######## Configuration

option(BUILD_BINARY "Build the CLI tool." ON)
option(BUILD_SHARED_LIBS "Build shared library and link executable to it." ON)
option(CHECK_SSE2 "Check and enable SSE2 extensions if supported. Disabling SSE on platforms which support it may decrease performances." ON)
set(TARGET_ARCHITECTURE "" CACHE STRING "Target CPU architecture. It is autodetected if not specified.")

if (BUILD_SHARED_LIBS)
	option(BUILD_STATIC "Build static library" ON)
endif (BUILD_SHARED_LIBS)

if (TARGET_ARCHITECTURE STREQUAL "")
    string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} TARGET_ARCHITECTURE)
endif (TARGET_ARCHITECTURE STREQUAL "")

if (TARGET_ARCHITECTURE MATCHES ".*(x86)|(amd).*")
    CHECK_C_COMPILER_FLAG(-msse2 SUPPORTS_CFLAG_SSE2)
    CHECK_C_COMPILER_FLAG(-mfpmath=sse SUPPORTS_CFLAG_SSE_MATH)
    if (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -mfpmath=sse")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mfpmath=sse")
    else (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffloat-store")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffloat-store")
    endif (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
endif (TARGET_ARCHITECTURE MATCHES ".*(x86)|(amd).*")

configure_file(
	uchardet.pc.in
	uchardet.pc
	@ONLY
)

install(
	FILES
		${CMAKE_BINARY_DIR}/uchardet.pc
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

######## Subdirectories

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(test)
