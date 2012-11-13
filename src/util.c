// hyper-cam - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on embedded Linux
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

#include "../headers/util.h"

// Print to stderror and exit the program
void exitWithError(const char* message)
{
	perror(message);
	exit(EXIT_FAILURE);
}

void xioctl(int fd, int request, void *arg)
{
	if (-1 == ioctl(fd, request, arg)) {
		exitWithError("");
	}
}

// Convert byte stream to integer
uint16_t byteToInt(byte bytes[2])
{
	uint16_t rv = 0;
	rv += (bytes[1] & 0xFF) << 8;
	rv += (bytes[0] & 0xFF);
	return rv;
}

int* prepareDataFile(const char* imagePath)
{
    // Create a buffer for the name of the image
    char out_name[256];
    // Set the name of the image
    sprintf(out_name, "%s/out.data", imagePath);
    // Create an output file
    int* fout = (int*)malloc(sizeof(int));
    *fout = open(out_name, O_CREAT | O_RDWR | O_NONBLOCK, 000777);
	if (*fout == -1) exitWithError(out_name);
	return fout;
}
