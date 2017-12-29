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
pkg_check_modules(GLESv2 glesv2 REQUIRED)

################################################################################
# Includes
################################################################################

################################################################################
# Sources
################################################################################

set(SOURCES
	main.cpp
	AglBeLauncher.cpp
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
	${GLESv2_LIBRARIES}
	${ILM_CTRL_LIBRARIES}
)
