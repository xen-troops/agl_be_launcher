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

#include <iostream>

#include <GLES2/gl2.h>

#include <xen/be/Exception.hpp>

#include "WlEglSurface.hpp"

using std::cout;
using std::endl;

using XenBackend::Exception;

/*******************************************************************************
 * WlEglSurface
 ******************************************************************************/

WlEglSurface::WlEglSurface() :
	mWlDisplay(nullptr),
	mWlRegistry(nullptr),
	mWlCompositor(nullptr),
	mWlIviApplication(nullptr),
	mWlSurface(nullptr),
	mWlIviSurface(nullptr),
	mWlEglWindow(nullptr),
	mEglDisplay(EGL_NO_DISPLAY),
	mEglConfig(0),
	mEglSurface(EGL_NO_SURFACE),
	mEglContext(EGL_NO_CONTEXT),
	mLog("WlEglSurface")
{
	try
	{
		init();
	}
	catch(const std::exception& e)
	{
		release();

		throw;
	}
}

WlEglSurface::~WlEglSurface()
{
	release();
}

/*******************************************************************************
 * Public
 ******************************************************************************/

void WlEglSurface::createSurface(uint32_t id, int width, int height)
{
	LOG(mLog, DEBUG) << "Create surface id: " << id;

	mWlSurface = wl_compositor_create_surface(mWlCompositor);

	if (!mWlSurface)
	{
		throw Exception("Can't create surface", errno);
	}

	mWlIviSurface = ivi_application_surface_create(mWlIviApplication,
												   id,
												   mWlSurface);

	if (!mWlIviSurface)
	{
		throw Exception("Can't create IVI surface", errno);
	}

	mWlEglWindow = wl_egl_window_create(mWlSurface, width, height);

	if (!mWlEglWindow)
	{
		throw Exception("Can't create EGL window", errno);
	}

	wl_display_flush(mWlDisplay);

	int nConfig;

	EGLint configAttribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_RED_SIZE,   8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE,  8,
			EGL_ALPHA_SIZE, 8,
			//EGL_SAMPLE_BUFFERS, 1,
			//EGL_SAMPLES,        2,
			EGL_NONE,
	};

	if (!eglChooseConfig(mEglDisplay, configAttribs, &mEglConfig, 1, &nConfig)
		|| (nConfig != 1))
	{
		throw Exception("Can't choose EGL config", ENOENT);
	}

	mEglSurface = eglCreateWindowSurface(mEglDisplay, mEglConfig,
										 (EGLNativeWindowType)mWlEglWindow,
										 nullptr);

	if (mEglSurface == EGL_NO_SURFACE)
	{
		throw Exception("Can't create EGL surface", errno);
	}

	EGLint contextAttribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE,
	};

	mEglContext = eglCreateContext(mEglDisplay, mEglConfig, nullptr,
								  contextAttribs);

	if (mEglContext == EGL_NO_CONTEXT)
	{
		throw Exception("Can't create EGL context", ENOENT);
	}

	if (!eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext))
	{
		throw Exception("Can't make EGL current", errno);
	}

	eglSwapBuffers(mEglDisplay, mEglSurface);

	wl_display_flush(mWlDisplay);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

void WlEglSurface::sRegistryHandler(void *data, wl_registry *registry, uint32_t id,
							   const char *interface, uint32_t version)
{
	static_cast<WlEglSurface*>(data)->registryHandler(registry, id, interface,
												 version);
}

void WlEglSurface::sRegistryRemover(void *data, struct wl_registry *registry,
							   uint32_t id)
{
	static_cast<WlEglSurface*>(data)->registryRemover(registry, id);
}

void WlEglSurface::registryHandler(wl_registry *registry, uint32_t id,
							  const std::string& interface, uint32_t version)
{
	LOG(mLog, DEBUG) << "Registry event, itf: " << interface << ", id: " << id
					 << ", version: " << version;

	if (interface == "wl_compositor")
	{
		mWlCompositor = static_cast<wl_compositor*>(wl_registry_bind(
				mWlRegistry, id, &wl_compositor_interface, version));
	}

	if (interface == "ivi_application")
	{
		mWlIviApplication = static_cast<ivi_application*>(wl_registry_bind(
				mWlRegistry, id, &ivi_application_interface, version));
	}
}

void WlEglSurface::registryRemover(wl_registry *registry, uint32_t id)
{
}

void WlEglSurface::init()
{
	LOG(mLog, DEBUG) << "Init";

	initWayland();
	initEGL();
}

void WlEglSurface::release()
{
	LOG(mLog, DEBUG) << "Release";

	releaseSurface();
	releaseEGL();
	releaseWayland();
}

void WlEglSurface::initWayland()
{
	mWlDisplay = wl_display_connect(nullptr);

	if (!mWlDisplay)
	{
		throw Exception("Can't connect to display", errno);
	}

	mWlRegistryListener = {sRegistryHandler, sRegistryRemover};

	mWlRegistry = wl_display_get_registry(mWlDisplay);

	if (!mWlRegistry)
	{
		throw Exception("Can't get registry", errno);
	}

	wl_registry_add_listener(mWlRegistry, &mWlRegistryListener, this);

	wl_display_dispatch(mWlDisplay);
	wl_display_roundtrip(mWlDisplay);

	if (!mWlCompositor)
	{
		throw Exception("Can't get compositor", ENOENT);
	}

	if (!mWlIviApplication)
	{
		throw Exception("Can't get ivi application", ENOENT);
	}
}

void WlEglSurface::releaseWayland()
{
	if (mWlCompositor)
	{
		wl_compositor_destroy(mWlCompositor);
	}

	if (mWlIviApplication)
	{
		ivi_application_destroy(mWlIviApplication);
	}

	if (mWlRegistry)
	{
		wl_registry_destroy(mWlRegistry);
	}

	if (mWlDisplay)
	{
		wl_display_flush(mWlDisplay);
		wl_display_disconnect(mWlDisplay);
	}
}

void WlEglSurface::initEGL()
{
	mEglDisplay = eglGetDisplay((EGLNativeDisplayType)(mWlDisplay));

	if (mEglDisplay == EGL_NO_DISPLAY)
	{
		throw Exception("Can't get EGL display", ENOENT);
	}

	EGLint major, minor;

	if (!eglInitialize(mEglDisplay, &major, &minor))
	{
		throw Exception("Can't get initialize EGL", ENOENT);
	}

	if (!eglBindAPI(EGL_OPENGL_ES_API))
	{
		throw Exception("Can't bind EGL API", ENOENT);
	}
}

void WlEglSurface::releaseEGL()
{
	if (mEglDisplay != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglTerminate(mEglDisplay);

		wl_display_flush(mWlDisplay);
	}
}

void WlEglSurface::releaseSurface()
{
	if (mWlEglWindow)
	{
		wl_egl_window_destroy(mWlEglWindow);
	}

	if (mWlIviSurface)
	{
		ivi_surface_destroy(mWlIviSurface);
	}

	if (mWlSurface)
	{
		wl_surface_destroy(mWlSurface);
	}

	if (mWlDisplay)
	{
		wl_display_flush(mWlDisplay);
	}
}
