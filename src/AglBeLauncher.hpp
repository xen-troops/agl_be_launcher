/*
 *  AglLauncher class
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

#ifndef SRC_AGLBELAUNCHER_HPP_
#define SRC_AGLBELAUNCHER_HPP_

#include <ilm/ilm_control.h>
#include <ilm/ilm_common.h>

#include "WlEglSurface.hpp"

class AglBeLauncher
{
public:
	AglBeLauncher();
	~AglBeLauncher();

private:
	t_ilm_surface mSurfaceId;
	t_ilm_layer mLayerId;
	WlEglSurface mSurface;
	bool mSurfaceCreated;
	bool mLayerCreated;
	XenBackend::Log mLog;

	static AglBeLauncher* mInstance;

	static void sCreateNotification(ilmObjectType object, t_ilm_uint id,
									t_ilm_bool created, void* data);
	void createNotification(ilmObjectType object, t_ilm_uint id,
							t_ilm_bool created);

	static void sLayerNotification(t_ilm_layer layer,
								   ilmLayerProperties* properties,
								   t_ilm_notification_mask mask);
	void layerNotification(ilmLayerProperties* properties,
						   t_ilm_notification_mask mask);

	static void sSurfaceNotification(t_ilm_surface surface,
									 ilmSurfaceProperties* properties,
									 t_ilm_notification_mask mask);
	void surfaceNotification(ilmSurfaceProperties* properties,
							 t_ilm_notification_mask mask);

	void init();
	void release();
};

#endif /* SRC_AGLBELAUNCHER_HPP_ */
