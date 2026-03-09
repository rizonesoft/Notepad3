######## Project settings
cmake_minimum_required(VERSION 3.1)
include(CheckCCompilerFlag)
set (PACKAGE_NAME uchardet)
project (${PACKAGE_NAME} CXX C)
enable_testing()

######## Package information
set (PACKAGE_URL https://www.freedesktop.org/wiki/Software/uchardet/)
set (PACKAGE_BUGREPORT https://gitlab.freedesktop.org/uchardet/uchardet/-/issues)
set (UCHARDET_VERSION_MAJOR 0)
set (UCHARDET_VERSION_MINOR 0)
set (UCHARDET_VERSION_REVISION 8)

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
    string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" TARGET_ARCHITECTURE)
endif (TARGET_ARCHITECTURE STREQUAL "")

if (TARGET_ARCHITECTURE MATCHES ".*(x86|amd|i686).*")
    CHECK_C_COMPILER_FLAG(-msse2 SUPPORTS_CFLAG_SSE2)
    CHECK_C_COMPILER_FLAG(-mfpmath=sse SUPPORTS_CFLAG_SSE_MATH)
    if (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse2 -mfpmath=sse")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse2 -mfpmath=sse")
    else (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ffloat-store")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffloat-store")
    endif (CHECK_SSE2 AND SUPPORTS_CFLAG_SSE2 AND SUPPORTS_CFLAG_SSE_MATH)
endif (TARGET_ARCHITECTURE MATCHES ".*(x86|amd|i686).*")

configure_file(
	uchardet.pc.in
	uchardet.pc
	@ONLY
)

install(
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/uchardet.pc
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}/pkgconfig
)

######## Subdirectories

add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(test)

######## Exported targets

install(
	EXPORT UchardetTargets
	FILE ${PACKAGE_NAME}-targets.cmake
	NAMESPACE ${PACKAGE_NAME}::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
)

export(
	EXPORT UchardetTargets
	FILE "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-targets.cmake"
	NAMESPACE ${PACKAGE_NAME}::
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	${PACKAGE_NAME}-config-version.cmake
	VERSION ${UCHARDET_VERSION}
	COMPATIBILITY AnyNewerVersion
)

configure_file(
	${PACKAGE_NAME}-config.cmake.in
	${PACKAGE_NAME}-config.cmake
	@ONLY
)

install (
	FILES
		"${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-config.cmake"
		"${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}-config-version.cmake"
	DESTINATION
		${CMAKE_INSTALL_LIBDIR}/cmake/${PACKAGE_NAME}
)
