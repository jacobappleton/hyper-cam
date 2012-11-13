// hyper-cam - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on embedded Linux
//
// Image processing functions.
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
//   Technology Organisation (DSTO) - www.dsto.defence.gov.au
// - German Aerospace Center (DLR) - www.dlr.de/en/
// - University of Southern Queensland (USQ) - www.usq.edu.au
// - BAE Systems - www.baesystems.com
// - Japan Aerospace Exploration Agency (JAXA) - www.jaxa.jp/index_e.html
// - University of Minnesota (UMN) - www.umn.edu
// - AIMTEK, Inc. - www.umn.edu
// - Australian Youth Aerospace Association (AYAA) - www.ayaa.com.au
// - Centro Italiano Ricerche Aerospaziali (CIRA) - www.cira.it/en
// - The University of Adelaide - www.adelaide.edu.au
// - Teakle Composites - www.cira.colostate.edu
// - The University of New South Wales (UNSW) - www.unsw.edu.au/
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <stdbool.h>
#include <string.h>
#include "../headers/util.h"
#include <jmorecfg.h>
#include <jpeglib.h>
#include <jconfig.h>

struct imgDetails
{
	unsigned int height;
	unsigned int width;
	unsigned int size;
};

void compressJpeg(FILE* outfile, byte* imgbuf, unsigned int cfactor,
		int rlen, int imgheight, int inputComponents);
byte* YUYVtoYUV(byte* yuyvValues, struct imgDetails det);

#endif
