// hyper-cam - Interfaces camera sensor with serial communications for
// autonomous image capture of scientific experiments on embedded Linux
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
// along with this program.  If not, see <www.gnu.org/licenses/>.
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
// Code built with inspiration from "Video Grabber example using libv4l -
// Part I. Video for Linux Two API Specification" by Mauro Carvalho Chehab
// (mchehab@infradead.org) available at:
// http://linuxtv.org/downloads/v4l-dvb-apis/v4l2grab-example.html
//
// You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

#define _GNU_SOURCE
#include "../headers/camera.h"

// Open the camera device as a file
void openDevice(const char* location, int* fd)
{
	*fd = open(location, O_RDWR, 0);
	if(*fd==1)
	{
		exitWithError("Open camera failed");
	}
	else
	{
		printf("Opened Camera");
	}
}

// Get the capabilities of the camera device
// as described here:
// http://linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html#device-capabi
// lities
void getCapabilities(int* fd)
{
	struct v4l2_capability cap;
	xioctl(*fd, VIDIOC_QUERYCAP, &cap);
	printf("Capabilities: %x\nDriver: %s\n", cap.capabilities, cap.driver);
}

// Close the camera device
void closeDevice(int* fd)
{
	if(*fd != -1) close(*fd);
	printf("Close device successful (%d)", *fd);
	*fd = -1;
	return;
}

// Get the format in which the camera driver provides frames from the camera
struct imgDetails getFrameFormat(int* fd)
{
	struct imgDetails det; // struct to hold image information
	struct v4l2_format fmt; // the v4l2 struct for frame format
	CLEAR(fmt); // clear this structs memory
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // we are performing video capture
    xioctl(*fd, VIDIOC_G_FMT, &fmt); // request the format from the driver
    det.width = fmt.fmt.pix.width; // set the image width for later on
    det.height = fmt.fmt.pix.height; // set the image height for later on
    det.size = det.width*det.height*3; // set the image size for later on
    return det;
}

// Get the struct to be sent to v4l2 to retrieve the image data
struct v4l2_requestbuffers getReqBufs(uint nbuffers, uint minbuffers, int* fd)
{
	struct v4l2_requestbuffers reqbuf;
	CLEAR(reqbuf); // Clear the required memory
	reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // we are capturing video
	reqbuf.memory = V4L2_MEMORY_MMAP; // map kernel-space memory to user-space
	reqbuf.count = nbuffers; // the number of frames we wish to retrieve
	xioctl(*fd, VIDIOC_REQBUFS, &reqbuf); // request the buffers
	if (reqbuf.count < minbuffers)
	{
		// Should never be w/out enough memory unless other proc is hogging
		printf("Not enough buffer memory - trying again\n");
	}
	return reqbuf;
}

// Map the buffer of image data from kernel to user space
void mapBuffer(const struct v4l2_requestbuffers reqbuf, int nbuffers,
		struct v4l2_buffer buf, struct buffer* buffers, int* fd)
{
	CLEAR(buf); // clear the v4l2 frame buffer struct
	buf.type = reqbuf.type; // we are capturing video
	buf.memory = V4L2_MEMORY_MMAP; // we are mapping from kernel-space to user
	buf.index = nbuffers; // the number of buffers requested
	xioctl(*fd, VIDIOC_QUERYBUF, &buf); // query v4l2 for these buffers
	// Set the length of the buffer pointer struct to the number of buffers
	// returned by v4l2
	buffers[nbuffers].length = buf.length;
	// Perform the mapping, with the pointer of the memory mapped stored in
	// the user-space buffer pointer struct
	buffers[nbuffers].start = (byte*)mmap(NULL, buf.length,
			PROT_READ | PROT_WRITE, MAP_SHARED, *fd, buf.m.offset);
	// Memory mapping should not fail is performed on a correct fp for a
	// camera. If mmap does fail in testing, reasons for failure available
	// here: http://pubs.opengroup.org/onlinepubs/009695399/functions/mmap.html
	if (MAP_FAILED == buffers[nbuffers].start)
	{
		exitWithError("Memory mapping failed.");
	}
}

// Queue a buffer capture in the V4L2 driver
void queueBuffer(int i, struct v4l2_buffer buf, int* fd) {
	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf.memory = V4L2_MEMORY_MMAP;
	buf.index = i;
	ioctl(*fd, VIDIOC_QBUF, &buf);
}

// Takes the image data from the buffer and places it in a singly-linked list
// node prepared for output via serial communication
void createImage(struct v4l2_buffer buf, struct buffer* buffers,
		const int i, struct lstnode* node, int* fd,
		struct imgDetails det, int cqual)
{
	byte* imageData = (byte*)calloc(AVG_IMG_SIZE, sizeof(byte)); // img data buf
	FILE* fout = fmemopen(imageData, AVG_IMG_SIZE, "wb"); // open buffer
    CLEAR(buf); // clear from the buffer object all previous settings applied
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE; // this is a video capture buffer
    buf.memory = V4L2_MEMORY_MMAP; // we are using kernel memory mapping
    ioctl(*fd, VIDIOC_DQBUF, &buf); // dequeue the buffer from v4l2 for reading
    byte* yuvBytes = YUYVtoYUV(buffers[i].start, det); // YUYV bytes to YUV
    // Compress these bytes to a JPEG image using libjpeg
	compressJpeg(fout, yuvBytes, cqual, det.width, det.height, 3);
	unsigned long int bytesWritten = ftell(fout); // # bytes written to file
	if(node->size > 0) // set the data for this node of the singly linked list
	{
		node->size = 0;
		free(node->img);
	}
	node->img = (byte*)malloc(sizeof(byte)*bytesWritten); // alloc image
	node->size = bytesWritten; // set the size of the node to the bytes written
	node->tstamp = time(NULL); // set the timestamp for the time image taken
	memcpy(node->img, imageData, node->size); // copy the data into the node
	fclose(fout); // close the image data memory file ptr
	free(imageData); // free the image data byte array
	free(yuvBytes); // free the yuyBytes data byte array
}

struct lstnode* writeDataToSerial(struct telpkt* req, int fd,
		struct lstnode* cNode, int nOffset)
{
	if (req->bytesRequested > 0 && cNode->size - nOffset > 0)
	{
		printf("Bytes requested: %d\n", req->bytesRequested);
		//printf(".");
		// Number of bytes requested is larger than image bytes remaining
		// Image "fits within buffer"
		if(req->bytesRequested >= cNode->size - nOffset)
		{
			struct telpkt* t = createOutputTelPkt(req->bytesRequested,
												  cNode->size - nOffset);
			// Copy number of bytes in image from image to telemetry packet
			memcpy(t->data, cNode->img + nOffset, cNode->size - nOffset);
			writeToUart(t, fd);
			// We've transmitted the whole image so start again
			nOffset = 0;
			cNode = cNode->next;
		}
		else // Number of bytes requested is smaller than image size
		{
			printf("Bytes requested: %d\n", req->bytesRequested);
			struct telpkt* t = createOutputTelPkt(req->bytesRequested,
												  req->bytesRequested);
			// Copy number of bytes in request from image to telemetry packet
			memcpy(t->data, cNode->img + nOffset, req->bytesRequested);
			// Encode the telemetry packet
			writeToUart(t, fd);
			// We have only transmitted part of the image so move imageBuffer
			// by the bytes requested
			cNode->img += req->bytesRequested;
			// Change image size to take away bytes requested
			cNode->size -= req->bytesRequested;
		}
	}
	return cNode;
}

// Writes image data to a file on the non-temporary storage (ie. HDD or flash
// memory) of the device running this sofware.
void* writeImageContentToFile(void* args)
{
	struct threadArgs* 	inputs = (struct threadArgs*)args;
	int 				nodeCtr = 0;
	struct lstnode* 	node = inputs->cNode;
	pthread_mutex_t* 	mutex = inputs->mutex;
	free(args);
	int fd = openPort(); 		        // open the serial port
	while(TRUE)
	{
		byte* buf = (byte*)malloc(6); // allocate buffer for meta data
		if(read(fd, buf, 6) == 6) // read meta data
		{
			pthread_mutex_lock(mutex); // lock so other thread can't change ptrs
			printf("Received: %s\n", buf);
			struct telpkt* tp = decode(buf, 5); // create telemetry pkt
			if(node->size > 0) node = writeDataToSerial(tp, fd, node, nodeCtr);
			free(tp);
			pthread_mutex_unlock(mutex);
		}
		free(buf);
	}
	free(inputs);
	pthread_exit(NULL);
}

// Turn on video streaming - this is the point at which the LED on the
// camera should light up
bool turnOnCamera(int* fd, bool streamOn, int* imageCaptureType)
{
	if (!streamOn) {
		ioctl(*fd, VIDIOC_STREAMON, imageCaptureType);
		streamOn = TRUE;
	}
	return streamOn;
}

void createThread(struct lstnode* cNode,
		pthread_mutex_t* mutex) {
	pthread_t thread;
	struct threadArgs* arg =
			(struct threadArgs*)malloc(sizeof(struct threadArgs));
	arg->cNode = cNode;
	arg->mutex = mutex;
	pthread_create(&thread, NULL, writeImageContentToFile, (void*) arg);
}

// Get a frame from the camera device
void getFrames(int noFrames, int minNoFrames, int* fd, int* imgCaptureType,
			   int cqual, int fps, int bufferSize)
{
	struct v4l2_buffer 			buf;
	struct buffer* 				bufs;
	struct v4l2_requestbuffers 	reqbuf = getReqBufs(noFrames, minNoFrames, fd);
	unsigned int 				n_buffers, i, ctr = 0;
    bool 						started = FALSE, streamOn = false;
    struct lstnode* cNode = allocate(bufferSize);
    pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutex, NULL);
	struct imgDetails det = getFrameFormat(fd); // get video format details
	bufs = (struct buffer*)calloc(reqbuf.count, sizeof(*bufs)); // alloc buffers
	// If allocation fails
	if (bufs == NULL) exitWithError("Could not allocate buffers.");
	// Map the memory in kernel space to user space to access video efficiently
	for (n_buffers = 0; n_buffers < reqbuf.count; n_buffers++)
	{
		mapBuffer(reqbuf, n_buffers, buf, bufs, fd);
	}
	// Queue the request struct to v4l2 to retrieve the video data mem location
	for(i = 0; i < reqbuf.count; i++)	queueBuffer(i, buf, fd);
	streamOn = turnOnCamera(fd, streamOn, imgCaptureType); // turn on camera
    while(true) //forever
    {
		if(!started) // if the write data thread has not yet been started
		{
			started = TRUE;
			createThread(cNode, mutex);
		}
		for(i = 0; i < reqbuf.count; i++) // for each frame returned
		{
			usleep(fps); // maintain steady frame rate
			pthread_mutex_lock(mutex); // lock pointers
			createImage(buf, bufs, i, cNode, fd, det, cqual);
			cNode = cNode->next; // work with the next node in the list
			printf("|%d|\n", ctr++);
			pthread_mutex_unlock(mutex); // release locks
			queueBuffer(i, buf, fd); // queue up a new buffer
		}
    }
    for (i = 0; i < reqbuf.count; ++i) // unmap memory
    {
    	munmap(bufs[i].start, bufs[i].length);
    }
    free(mutex);
    free(cNode);
    free(bufs);
}

int main(int argc, char *argv[]) {
	int 		*fd = (int*)malloc(sizeof(int)), qual = 0, fps = 0, bufSize = 0;
	char 		*camera;
	if(argc != 5) 						// Check correct number of args
	{
		char errorMsg[256];
		sprintf(errorMsg, "usage: %s cameraDevice JpegQuality fps bufferSize",
				argv[0]);
		exitWithError(errorMsg);
	}
	else
	{
		camera = argv[1];	// camera location
		// Image quality
		if(atoi(argv[2]) > 0 && atoi(argv[2]) <= 100) qual = atoi(argv[2]);
		else exitWithError("Set JpegQuality between 1 and 100 inclusive.");
		// frame delay
		if(atoi(argv[3]) > 0) fps = atoi(argv[3]);
		// list size
		if(atoi(argv[4]) > 0) bufSize = atoi(argv[4]);
	}
	printf("Camera Interface  Copyright (C) 2012  Jacob Appleton\n\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY;\n");
    printf("This is free software, and you are welcome to redistribute it \n");
    printf("under certain conditions.\n");
    printf("Visit http://www.gnu.org/licenses/gpl.html for more details.\n\n");
	*fd = -1;
	openDevice(camera, fd);
	getCapabilities(fd);
	int* imageCaptureType = (int*)malloc(sizeof(int));
	*imageCaptureType = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	getFrames(1, 1, fd, imageCaptureType, qual, fps, bufSize);
    // Turn the stream off - this will turn off the camera's LED light
    ioctl(*fd, VIDIOC_STREAMOFF, imageCaptureType);
	closeDevice(fd);
	free(fd);
	free(imageCaptureType);
	pthread_exit(NULL);
	return 0;
}

