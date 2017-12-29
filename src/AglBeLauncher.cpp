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

using std::this_thread::sleep_for;
using std::thread;
using std::chrono::milliseconds;

using XenBackend::Exception;

/*******************************************************************************
 * AglBeLauncher
 ******************************************************************************/

AglBeLauncher::AglBeLauncher() :
	mSurfaceId(50),
	mLayerId(102 * 100000 + getpid()),
	mTerminate(false),
	mIsOnTop(false),
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

void AglBeLauncher::init()
{
	LOG(mLog, DEBUG) << "Init";

	LOG(mLog, DEBUG) << "Expected layer: " << mLayerId;

	auto ret = ilm_init();

	if (ret != ILM_SUCCESS)
	{
		throw Exception("Can't initialize ilm", ret);
	}

	mSurface.createSurface(mSurfaceId, 1080, 1487);

	mThread = thread(&AglBeLauncher::run, this);
}

void AglBeLauncher::release()
{
	LOG(mLog, DEBUG) << "Release";

	mTerminate = true;

	if (mThread.joinable())
	{
		mThread.join();
	}
//	ilm_destroy();
}

void AglBeLauncher::run()
{
	while(!mTerminate)
	{
		sleep_for(milliseconds(100));

		 t_ilm_int length;
		 t_ilm_layer* layers;

		if (ilm_getLayerIDsOnScreen(0, &length, &layers) == ILM_SUCCESS)
		{
			if (length)
			{
				if (layers[length - 1] == mLayerId && !mIsOnTop)
				{
					mIsOnTop = true;

					LOG(mLog, DEBUG) << "Show";
				}
				else if (layers[length - 1] != mLayerId && mIsOnTop)
				{
					mIsOnTop = false;

					LOG(mLog, DEBUG) << "Hide";
				}
			}

			free(layers);
		}
	}
}
