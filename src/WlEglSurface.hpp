/*
 *  WlEglSurface class
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * Copyright (C) 2016 EPAM Systems Inc.
 *
 */

#ifndef SRC_WLEGLSURFACE_HPP_
#define SRC_WLEGLSURFACE_HPP_

#include <string>

#include <wayland-client.h>
#include <ilm/ivi-application-client-protocol.h>
#include <EGL/egl.h>
#include <wayland-egl.h>

#include <xen/be/Log.hpp>

class WlEglSurface
{
public:

	WlEglSurface();
	~WlEglSurface();

	void createSurface(uint32_t id, int width, int height);

private:

	wl_display* mWlDisplay;
	wl_registry* mWlRegistry;
	wl_registry_listener mWlRegistryListener;
	wl_compositor* mWlCompositor;
	ivi_application* mWlIviApplication;
	wl_surface* mWlSurface;
	ivi_surface* mWlIviSurface;
	wl_egl_window* mWlEglWindow;
	EGLDisplay mEglDisplay;
	EGLConfig mEglConfig;
	EGLSurface mEglSurface;
	EGLContext mEglContext;
	XenBackend::Log mLog;

	static void sRegistryHandler(void *data, wl_registry *registry,
								 uint32_t id, const char *interface,
								 uint32_t version);
	static void sRegistryRemover(void *data, struct wl_registry *registry,
								 uint32_t id);

	void registryHandler(wl_registry *registry, uint32_t id,
						 const std::string& interface, uint32_t version);
	void registryRemover(wl_registry *registry, uint32_t id);

	void init();
	void release();

	void initWayland();
	void releaseWayland();

	void initEGL();
	void releaseEGL();

	void releaseSurface();

};

#endif /* SRC_WLEGLSURFACE_HPP_ */
