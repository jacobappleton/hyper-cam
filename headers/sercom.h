// Flight Camera - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on an embedded Linux
//
// Serial port communication functions.
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
// Built with inspiration from "Serial Programming HOWTO" by Gary Frerking
// (gary@frerking.org) and Peter Baumann available at:
// http://tldp.org/HOWTO/Serial-Programming-HOWTO/
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#ifndef SERIAL_COMMUNICATION_H
	#define SERIAL_COMMUNICATION_H

	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/time.h>
	#include <sys/select.h>
	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <stdbool.h>
	#include <string.h>
	#include <stdint.h>
	#include "../headers/util.h"
	#include <linux/serial.h>
	// ------------------------------------------------------------------------
	// TO CHANGE BAUD RATE, MODIFY THIS MACRO
	#ifndef BAUDRATE
		#define BAUDRATE B460800
	#endif
	// ------------------------------------------------------------------------
	// ------------------------------------------------------------------------
	// TO CHANGE SERIAL PORT, MODIFY THIS MACRO
	#ifndef MODEMDEVICE
		#define MODEMDEVICE "/dev/ttyS0"
	#endif
	// ------------------------------------------------------------------------

	#ifndef TELEMETRY_HEADER
		#define TELEMETRY_HEADER 0xAA
	#endif

	struct telpkt
	{
		uint16_t bytesRequested;
		uint16_t bytesContained;
		byte* data;
		byte xor;
		byte* output;
		byte* outputStart;
		size_t outputSize;
	};

	int openPort();
	int openPortFd();
	void encode(struct telpkt* t);
	void cleanUp(struct telpkt* t);
	struct telpkt* decode(byte inputStream[6], unsigned int inputSize);
	struct telpkt* getTp(int16_t req, int16_t con);
	struct telpkt* createOutputTelPkt(uint16_t bRequested, uint16_t bContained);
	void writeToUart(struct telpkt* t, int fd);

#endif
