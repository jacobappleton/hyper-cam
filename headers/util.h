// Flight Camera - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on an embedded Linux
//
// General utility functions.
//
// Copyright (C) 2012 Jacob Appleton
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// This software was developed as part of the Scramspace I Flight Experiment,
// funded by the Australian Space Research Program -
// http://www.space.gov.au/AUSTRALIANSPACERESEARCHPROGRAM and involving:
// - The University of Queensland (UQ) - www.uq.edu.au
// - Australian Government Department of Defence - Defence Science and
//   Technology Organisation (DSTO) - http://www.dsto.defence.gov.au
// - German Aerospace Center (DLR) - http://www.dlr.de/en/
// - University of Southern Queensland (USQ) - www.usq.edu.au
// - BAE Systems - www.baesystems.com
// - Japan Aerospace Exploration Agency (JAXA) - www.jaxa.jp/index_e.html
// - University of Minnesota (UMN) - www.umn.edu
// - AIMTEK, Inc. - www.umn.edu
// - Australian Youth Aerospace Association (AYAA) - www.ayaa.com.au
// - Centro Italiano Ricerche Aerospaziali (CIRA) - www.cira.it/en
// - The University of Adelaide - www.adelaide.edu.au
// - Teakle Composites - http://www.cira.colostate.edu
// - The University of New South Wales (UNSW) - www.unsw.edu.au/
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#ifndef UTILITIES_H
	#define UTILITIES_H

	#include <string.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/ioctl.h>
	#include <stdint.h>
	#include <unistd.h>
	#include <fcntl.h>

	// Macro to clear a pointer to memory
	#ifndef CLEAR
	#define CLEAR(x) memset(&(x), 0, sizeof(x))
	#endif

	// Macro to clear a pointer to memory
	#ifndef CLEAR
		#define CLEAR(x) memset(&(x), 0, sizeof(x))
	#endif

	#ifndef AVG_IMG_SIZE
		#define AVG_IMG_SIZE 20000
	#endif

	typedef unsigned char byte;
	typedef unsigned int uint;

	// Contains details about image capture pointer in user-space memory
	struct buffer
	{
		unsigned char *start;
		size_t length;
	};

	struct node
	{
		byte* img;
		size_t size;
		struct node* next;
	};

	void exitWithError(const char* message);
	void xioctl(int fd, int request, void *arg);
	uint16_t byteToInt(byte bytes[2]);

#endif
