# hyper-cam - Interfaces camera sensor with serial communications for
# autonomous image capture of scientific experiments on embedded Linux
#
# Copyright (C) 2012 Jacob Appleton
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http:#www.gnu.org/licenses/>.
#
# This software was developed as part of the Scramspace Flight Experiment I,
# funded by the Australian Space Research Program and involving:
# - The University of Queensland (UQ) - www.uq.edu.au
# - Australian Government Department of Defence - Defence Science and
#   Technology Organisation (DSTO) - http:#www.dsto.defence.gov.au
# - German Aerospace Center (DLR) - http:#www.dlr.de/en/
# - University of Southern Queensland (USQ) - www.usq.edu.au
# - BAE Systems - www.baesystems.com
# - Japan Aerospace Exploration Agency (JAXA) - www.jaxa.jp/index_e.html
# - University of Minnesota (UMN) - www.umn.edu
# - AIMTEK, Inc. - www.umn.edu
# - Australian Youth Aerospace Association (AYAA) - www.ayaa.com.au
# - Centro Italiano Ricerche Aerospaziali (CIRA) - www.cira.it/en
# - The University of Adelaide - www.adelaide.edu.au
# - Teakle Composites - http:#www.cira.colostate.edu
# - The University of New South Wales (UNSW) - www.unsw.edu.au/
#
# You can contact Jacob Appleton via email at: jacob.appleton@uqconnect.edu.au

camera:		camera.o imgproc.o sercom.o util.o lnklst.o
		gcc -ggdb camera.o imgproc.o sercom.o util.o lnklst.o -o camera -ljpeg -lpthread

camera.o:	src/camera.c	headers/imgproc.h headers/sercom.h headers/util.h
			gcc -ggdb -Wall -c src/camera.c -o camera.o 

imgproc.o:	src/imgproc.c	headers/imgproc.h
					gcc -ggdb -Wall -c src/imgproc.c -o imgproc.o 
			
sercom.o:	src/sercom.c headers/sercom.h
			gcc -ggdb -Wall -c src/sercom.c -o sercom.o
				
util.o:		src/util.c headers/util.h
			gcc -ggdb -Wall -c src/util.c -o util.o
					
lnklst.o:	src/lnklst.c headers/lnklst.h
			gcc -ggdb -Wall -c src/lnklst.c -o lnklst.o
