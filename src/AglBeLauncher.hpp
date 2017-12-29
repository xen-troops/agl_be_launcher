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

#include <atomic>
#include <thread>

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
	std::atomic_bool mTerminate;
	bool mIsOnTop;
	XenBackend::Log mLog;

	std::thread mThread;

	void init();
	void release();

	void run();
};

#endif /* SRC_AGLBELAUNCHER_HPP_ */
