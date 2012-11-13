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
// Make sure when building to add the -ljpeg flag to the linker.
// Code built with inspiration from "example.c" from the libjpeg API
// documentation available at:
// http://libjpeg-turbo.svn.sourceforge.net/viewvc/libjpeg-turbo/trunk/example.c
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#include "../headers/imgproc.h"
#include "../headers/jpeglib.h"

// Set the details of the image being compressed in the compression manager
// struct
void setImgDetails(int rlen, int imgheight, int inputComponents,
		unsigned int cfactor, j_compress_ptr cinfo) {
	//Set image source details
	cinfo->image_width = rlen;
	cinfo->image_height = imgheight;
	cinfo->input_components = inputComponents;
	cinfo->in_color_space = JCS_YCbCr;
	jpeg_set_defaults(cinfo); // set the rest to defaults
	jpeg_set_quality(cinfo, cfactor, TRUE); // set compression factor
}

// Create the JPEG compression manager struct to handle running the libjpeg
// compression
void createJpegMgr(j_compress_ptr cinfo, FILE* outfile) {
	jpeg_create_compress(cinfo);
	jpeg_stdio_dest(cinfo, outfile); // set image source details
}

// Takes RAW image data (unsigned char*), generates and saves a compressed
// JPEG image.
// compressionFactor scales from 0 to 100. 0 = worst quality,
// 										   100 = best quality
void compressJpeg(FILE* outfile, byte* imgbuf, unsigned int cfactor,
		int rlen, int imgheight, int incomponents)
{
	struct jpeg_compress_struct 	cinfo;  // compression manager struct
	struct jpeg_error_mgr 			jerr;   // error manager struct
	byte* 							rowptr[imgheight]; // a row of the image
	unsigned int					stride = 0;
	if(cfactor < 0 || cfactor > 100) // ensure compression between 0 and 100
	{
		exitWithError("Compression factor must be between 0 and 100.");
	}
	createJpegMgr(&cinfo, outfile); // create JPEG compression manager
	cinfo.err = jpeg_std_error(&jerr); // use jerr struct on error
	setImgDetails(rlen, imgheight, incomponents, cfactor, &cinfo);
	jpeg_start_compress(&cinfo, TRUE); // Start the compression
	if(stride == 0) stride = rlen * incomponents; // alloc 1 row arr of JSAMPLE
	while (cinfo.next_scanline < cinfo.image_height)
	{
		rowptr[cinfo.next_scanline] = &imgbuf[cinfo.next_scanline*stride];
		jpeg_write_scanlines(&cinfo, &rowptr[cinfo.next_scanline], 1);
	}
	jpeg_finish_compress(&cinfo); // Finish the compression
	jpeg_destroy_compress(&cinfo); // Free all memory used by libjpeg
}

byte* YUYVtoYUV(byte* yuyvValues, struct imgDetails det)
{
	unsigned int j, yuvByteCount; // Create a buffer to hold the YUV image
	byte* yuvOutput = (byte*)malloc(det.size);
    yuvByteCount = 0;
	for(j = 0; j < (((det.width * det.height)>>1) * 4); j+=4)
	{
		// Convert this pixel to YUV
		yuvOutput[0 + yuvByteCount] = yuyvValues[j]; //y0 = y
		yuvOutput[1 + yuvByteCount] = yuyvValues[j+1]; //u0 = u
		yuvOutput[2 + yuvByteCount] = yuyvValues[j+3]; //v0 = v
		yuvOutput[3 + yuvByteCount] = yuyvValues[j+2]; //y0 = y'
		yuvOutput[4 + yuvByteCount] = yuyvValues[j+1]; //u0 = u
		yuvOutput[5 + yuvByteCount] = yuyvValues[j+3]; //v0 = v
		yuvByteCount += 6;
	}
	return yuvOutput;
}
