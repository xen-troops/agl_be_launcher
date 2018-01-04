################################################################################
# Check packages
################################################################################

include(FindPkgConfig)

pkg_check_modules (WAYLAND REQUIRED wayland-client)

pkg_check_modules (IVI_APP ivi-application)
if(NOT IVI_APP_FOUND)
	set(IVI_APP_LIBRARIES ivi-application)
endif()

pkg_check_modules(ILM_CTRL ilmControl)
if(NOT ILM_CTRL_FOUND)
	set(ILM_CTRL_LIBRARIES "ilmControl;ilmCommon")
endif()

pkg_check_modules(EGL egl REQUIRED)
pkg_check_modules(WAYLAND_EGL wayland-egl REQUIRED)

pkg_check_modules(DBUS1 REQUIRED dbus-1)
pkg_check_modules(DBUS_CPP REQUIRED dbus-cpp)
pkg_check_modules(PROPERTIES_CPP REQUIRED properties-cpp)

################################################################################
# CFLAGS
################################################################################

string(REPLACE ";" " " DBUS1_CFLAGS_STR "${DBUS1_CFLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DBUS1_CFLAGS_STR}")

################################################################################
# Includes
################################################################################

################################################################################
# Sources
################################################################################

set(SOURCES
	main.cpp
	SurfaceSwitcher.cpp
	WlEglSurface.cpp
)

################################################################################
# Targets
################################################################################

add_executable(${PROJECT_NAME} ${SOURCES})

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

################################################################################
# Libraries
################################################################################

target_link_libraries(${PROJECT_NAME}
	${WAYLAND_LIBRARIES}
	${IVI_APP_LIBRARIES}
	${WAYLAND_EGL_LIBRARIES}
	${EGL_LIBRARIES}
	${ILM_CTRL_LIBRARIES}
	${DBUS_CPP_LIBRARIES}
	pthread
)

################################################################################
# Package
################################################################################

if(WITH_WGT)
	set_target_properties(${PROJECT_NAME} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/package/root/bin
	)
	
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/package/config.xml
			${CMAKE_BINARY_DIR}/package/root/config.xml
		COMMAND ${CMAKE_COMMAND} -E copy
			${CMAKE_SOURCE_DIR}/package/icon.png
			${CMAKE_BINARY_DIR}/package/root/icon.png
		COMMAND wgtpkg-pack -f -o
			${CMAKE_BINARY_DIR}/package/${PROJECT_NAME}.wgt
			${CMAKE_BINARY_DIR}/package/root/
	)
endif()
