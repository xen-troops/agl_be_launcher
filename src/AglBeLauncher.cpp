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

#include "AglBeLauncher.hpp"

#include <unistd.h>

#include <xen/be/Exception.hpp>

using XenBackend::Exception;

AglBeLauncher* AglBeLauncher::mInstance = nullptr;

/*******************************************************************************
 * AglBeLauncher
 ******************************************************************************/

AglBeLauncher::AglBeLauncher() :
	mSurfaceId(50),
	mLayerId(102 * 100000 + getpid()),
	mSurfaceCreated(false),
	mLayerCreated(false),
	mLog("AglBeLauncher")
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

AglBeLauncher::~AglBeLauncher()
{
	release();
}

/*******************************************************************************
 * Private
 ******************************************************************************/

void AglBeLauncher::sCreateNotification(ilmObjectType object, t_ilm_uint id,
										t_ilm_bool created, void* data)
{
	mInstance->createNotification(object, id, created);
}

void AglBeLauncher::createNotification(ilmObjectType object, t_ilm_uint id,
									   t_ilm_bool created)
{
	if (object == ILM_SURFACE && id == mSurfaceId && created)
	{
		LOG(mLog, DEBUG) << "Surface created";

		auto ret = ilm_surfaceAddNotification(mSurfaceId, sSurfaceNotification);

		if (ret == ILM_SUCCESS)
		{
			mSurfaceCreated = true;
		}
		else
		{
			LOG(mLog, ERROR) << "Can't register surface notification";
		}
	}

	if (object == ILM_LAYER && id == mLayerId && created)
	{
		LOG(mLog, DEBUG) << "Layer created";

		auto ret = ilm_layerAddNotification(mLayerId, sLayerNotification);

		if (ret == ILM_SUCCESS)
		{
			mLayerCreated = true;
		}
		else
		{
			LOG(mLog, ERROR) << "Can't register layer notification";
		}
	}
}

void AglBeLauncher::sLayerNotification(t_ilm_layer layer,
									   ilmLayerProperties* properties,
									   t_ilm_notification_mask mask)
{
	mInstance->layerNotification(properties, mask);
}

void AglBeLauncher::layerNotification(ilmLayerProperties* properties,
									  t_ilm_notification_mask mask)
{
	LOG(mLog, DEBUG) << "Layer notification: " << mask;
}

void AglBeLauncher::sSurfaceNotification(t_ilm_surface surface,
										 ilmSurfaceProperties* properties,
										 t_ilm_notification_mask mask)
{
	mInstance->surfaceNotification(properties, mask);
}

void AglBeLauncher::surfaceNotification(ilmSurfaceProperties* properties,
										t_ilm_notification_mask mask)
{
	LOG(mLog, DEBUG) << "Surface notification: " << mask;

	if (mask & ILM_NOTIFICATION_VISIBILITY)
	{
		LOG(mLog, DEBUG) << "Surface visibility: " << properties->visibility;
	}
}

void AglBeLauncher::init()
{
	LOG(mLog, DEBUG) << "Init";

	if (mInstance)
	{
		throw Exception("Only one instance of launcher is allowed", EEXIST);
	}


	LOG(mLog, DEBUG) << "Expected layer: " << mLayerId;

	mInstance = this;

	auto ret = ilm_init();

	if (ret != ILM_SUCCESS)
	{
		throw Exception("Can't initialize ilm", ret);
	}

	ret = ilm_registerNotification(sCreateNotification, nullptr);

	if (ret != ILM_SUCCESS)
	{
		throw Exception("Can't register notification", ret);
	}

	mSurface.createSurface(mSurfaceId, 1080, 1487);
}

void AglBeLauncher::release()
{
	LOG(mLog, DEBUG) << "Release";

	if (mSurfaceCreated)
	{
		ilm_surfaceRemoveNotification(mSurfaceId);
	}

	if (mLayerCreated)
	{
		ilm_layerRemoveNotification(mSurfaceId);
	}

	ilm_unregisterNotification();

//	ilm_destroy();
}
