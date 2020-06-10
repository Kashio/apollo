set(headers
	"${apollo_SOURCE_DIR}/include/apollo/apollo.h"
	"${apollo_SOURCE_DIR}/include/apollo/registry.h"
	"${apollo_SOURCE_DIR}/include/apollo/component.h"
	"${apollo_SOURCE_DIR}/include/apollo/component_storage.h"
	"${apollo_SOURCE_DIR}/include/apollo/archetype.h"
	"${apollo_SOURCE_DIR}/include/apollo/core/common.h"
	"${apollo_SOURCE_DIR}/include/apollo/core/type_traits.h")

add_library(apollo INTERFACE)

target_sources(apollo INTERFACE $<BUILD_INTERFACE:${headers}>)

target_include_directories(apollo INTERFACE
                       $<BUILD_INTERFACE:${apollo_SOURCE_DIR}/include>
                       $<INSTALL_INTERFACE:${APOLLO_INC_INSTALL_DIR}>
					   )

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${apollo_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
	"${project_config}"
	INSTALL_DESTINATION ${APOLLO_CMAKE_CONFIG_INSTALL_DIR})
write_basic_package_version_file("${version_config}" COMPATIBILITY SameMajorVersion)

if (APOLLO_ENABLE_INSTALL)
	install(TARGETS apollo
			EXPORT ${targets_export_name}
			ARCHIVE DESTINATION ${APOLLO_ARCHIVE_INSTALL_DIR}
			LIBRARY DESTINATION ${APOLLO_LIBRARY_INSTALL_DIR}
			RUNTIME DESTINATION ${APOLLO_RUNTIME_INSTALL_DIR}
			FRAMEWORK DESTINATION ${APOLLO_FRAMEWORK_INSTALL_DIR})

	install(
		DIRECTORY "${APOLLO_SOURCE_DIR}/include/apollo"
		DESTINATION ${APOLLO_INC_INSTALL_DIR}
		FILES_MATCHING PATTERN "*.*h")

	install(
      FILES "${project_config}" "${version_config}"
      DESTINATION "${APOLLO_CMAKE_CONFIG_INSTALL_DIR}")

	install(
		EXPORT ${targets_export_name}
		DESTINATION ${APOLLO_CMAKE_CONFIG_INSTALL_DIR})
endif()
