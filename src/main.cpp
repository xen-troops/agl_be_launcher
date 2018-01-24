/*
 *  AGL backend launcher
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
 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <iostream>
#include <csignal>

#include <execinfo.h>
#include <getopt.h>
#include <unistd.h>

#include "SurfaceSwitcher.hpp"

using std::cout;
using std::endl;
using std::string;

using XenBackend::Log;
using XenBackend::LogLevel;

/*******************************************************************************
 *
 ******************************************************************************/

void segmentationHandler(int sig)
{
	void *array[20];
	size_t size;

	LOG("Main", ERROR) << "Segmentation fault!";

	size = backtrace(array, 20);

	backtrace_symbols_fd(array, size, STDERR_FILENO);

	raise(sig);
}

void registerSignals()
{
	struct sigaction act {};

	act.sa_handler = segmentationHandler;
	act.sa_flags = SA_RESETHAND;

	sigaction(SIGSEGV, &act, nullptr);
}

void waitSignals()
{
	sigset_t set;
	int signal;

	sigemptyset(&set);
	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGTERM);
	sigprocmask(SIG_BLOCK, &set, nullptr);

	sigwait(&set,&signal);
}

int main(int argc, char *argv[])
{
	try
	{
		registerSignals();

		Log::setLogLevel(LogLevel::logDEBUG);

		SurfaceSwitcher switcher;

		waitSignals();
	}
	catch(const std::exception& e)
	{
		LOG("Main", ERROR) << e.what();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
