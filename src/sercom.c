// hyper-cam - Interfaces camera sensor with serial communications for
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

#include "../headers/sercom.h"

int openPortFd() {
	int		comFd;
	/*
	 Open modem device for reading and writing and not as controlling tty
	 because we don't want to get killed if linenoise sends CTRL-C.
	 */
	comFd = open(MODEMDEVICE, O_RDWR | O_NOCTTY);
	if (comFd < 0) {
		perror(MODEMDEVICE);
		exit(-1);
	}
	return comFd;
}

int openPort()
{
	int 	fd;					// the file descriptor for the serial port file
	struct 	termios tio, oldtio;		// struct for serial-coms
	// Open modem device for reading and writing and not as controlling tty
	// because we don't want to get killed if linenoise sends CTRL-C.
	fd = openPortFd();
	//memset(&tio, 0, sizeof(tio)); // clear struct for new port settings
    tcgetattr(fd,&oldtio); /* save current port settings */
    bzero(&tio, sizeof(tio));
	// Control mode flags:
	// BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
	// CRTSCTS : output hardware flow control (only used if the cable has
	//	    	 all necessary lines.)
	// CS8     : 8n1 (8bit,no parity,1 stopbit)
	// CLOCAL  : local connection, no modem contol
	// CREAD   : enable receiving characters
	tio.c_cflag =  BAUDRATE | CS8 | CLOCAL | CREAD;
	// Input mode flags:
	// IGNPAR  : ignore bytes with parity errors
    tio.c_iflag = IGNPAR;
	// ICRNL   : map CR to NL (otherwise a CR input on the other computer
	//	 		 will not terminate input)
	//tio.c_iflag = IGNPAR | ICRNL;
	tio.c_oflag = 0; // raw output
	tio.c_cc[VTIME] = 0;
	tio.c_cc[VMIN] = 6;
	tcflush(fd, TCIFLUSH); // clear modem lines
	tcsetattr(fd,TCSANOW,&tio); // set attributes
	return fd;
}

byte calcXor(byte* data, int size)
{
	byte output = 0x00;
	int i;
	for(i = 0; i < size; i++)
	{
		output = output ^ data[i];
	}
	return output;
}

void encode(struct telpkt* t)
{
	unsigned int	outputSize;
	outputSize = 6 + t->bytesContained;
	byte* output = (byte*)calloc(outputSize, 1); // alloc memory for output stream
	byte* outputStart = output; // create ptr to start of output stream
	output[0] = 0xAA; // as per standard, set first byte to 0x00
	output[1] = (t->bytesRequested >> 8) & 0xFF;
	output[2] = t->bytesRequested & 0xFF;
	output[3] = (t->bytesContained >> 8) & 0xFF;
	output[4] = t->bytesContained & 0xFF;
	output += 5;
	// Copy the data to the output
	memcpy(output, t->data, t->bytesContained);
	// Move the output by the number of bytes in the data
	output += t->bytesContained;
	// For everything in the output so far, XOR
	t->xor = calcXor(outputStart, outputSize - 1); // add checksum to the end
	*output = t->xor;
	t->output = outputStart; // set the pointer to the output data
	t->outputSize = outputSize; // set the size of the output data
}

// Little endian int to byte*
// eg: 1111111100000000 (0xFF 0x00)
// Little endian means reads int right to left
// So, shift right by 8 and we now have
// 0000000011111111 and pull off last 8 bits (0xFF)
// now take last 8 bits from non-shifted (0x00) (shifting works like +)
// now storing in byte order 1111111100000000
// 0xFF is a bit mask, probably just being safe as AFAIK c fills other
// bits with 0

struct telpkt* decode(byte inputStream[6], unsigned int inputSize)
{
	struct telpkt* t = (struct telpkt*)malloc(sizeof(struct
			telpkt)); // create telemetry packet
	byte req[2];
	if(inputStream[0] == 0xAA && calcXor(inputStream, inputSize) ==
			inputStream[5]) // if inputstream is valid
	{
		req[0] = inputStream[2];
		req[1] = inputStream[1];
		t->bytesRequested = byteToInt(req);  // set bytes requested
		t->bytesContained = 0;
		t->xor = calcXor(inputStream, inputSize);    // calculate checksum
	}
	else
	{
		// Error case - set no bytes requested so doesn't try to decode garbage
		t->bytesRequested = 0;
	}
	return t;
}

void cleanUp(struct telpkt* t)
{
	free(t->output);
	free(t);
}

// Create a new telemetry stream packet with the set bytes requested and
// bytes contained values
struct telpkt* getTp(int16_t req, int16_t con)
{
	struct telpkt* tp = (struct telpkt*)malloc(sizeof(struct telpkt));
	tp->bytesRequested = req;
	return tp;
}

// Creates a new telemetry stream packet with memory allocated for output image
// data
struct telpkt* createOutputTelPkt(uint16_t bRequested, uint16_t bContained)
{
	struct telpkt* t = (struct telpkt*) malloc(sizeof(struct telpkt));
	t->bytesRequested = bRequested;
	t->bytesContained = bContained;
	t->data = (byte*) malloc(bContained);
	return t;
}

// Writes data to the serial port
void writeToUart(struct telpkt* t, int fd)
{
	encode(t); // encode the telemetry packet
	write(fd, t->output, t->outputSize); // write encoded data to the serial
	fsync(fd); // flush buffer
	free(t->data); // free the data
	cleanUp(t); // cleanup the packet
}
