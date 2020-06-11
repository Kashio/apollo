cmake_minimum_required(VERSION 3.12.0)

project(apollo
	VERSION 1.0.0
	DESCRIPTION "C++ ECS"
	LANGUAGES CXX)

option(APOLLO_ENABLE_TESTING "Enable testing of the apollo library." ON)
option(APOLLO_ENABLE_INSTALL "Enable installation of apollo. (Projects embedding benchmark may want to turn this OFF.)" ON)

list(APPEND CMAKE_MODULE_PATH "${apollo_SOURCE_DIR}/cmake")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(APOLLO_INC_INSTALL_DIR "include")
	set(APOLLO_RUNTIME_INSTALL_DIR "bin")
	set(APOLLO_LIBRARY_INSTALL_DIR "lib")
	set(APOLLO_ARCHIVE_INSTALL_DIR "lib")
	set(APOLLO_FRAMEWORK_INSTALL_DIR "lib")
	set(APOLLO_CMAKE_CONFIG_INSTALL_DIR "lib/apollo/cmake")
	set(APOLLO_ADDITIONAL_FILES_INSTALL_DIR "lib/apollo")
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	include(GNUInstallDirs)
	set(APOLLO_INC_INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}")
	set(APOLLO_RUNTIME_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
	set(APOLLO_LIBRARY_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
	set(APOLLO_ARCHIVE_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
	set(APOLLO_FRAMEWORK_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}")
	set(APOLLO_CMAKE_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/apollo/cmake")
	set(APOLLO_ADDITIONAL_FILES_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/apollo")
else()
	message(FATAL_ERROR "Could not set install folders for this platform!")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_EXTENSIONS OFF)

	set_property(GLOBAL PROPERTY USE_FOLDERS ON)

	find_package(Doxygen)
	if(Doxygen_FOUND)
		add_subdirectory(docs)
	else()
		message(STATUS "Doxygen not found, not building docs")
	endif()
endif()

include(FetchContent)
if(${CMAKE_VERSION} VERSION_LESS 3.14)
	include(add_FetchContent_MakeAvailable)
endif()

add_subdirectory(src)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME) AND APOLLO_ENABLE_TESTING)
	enable_testing()
	add_subdirectory(test)
endif()
