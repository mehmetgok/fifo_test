#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <string.h>
#include <signal.h> /* Ctrl+C Handling */

#include <unistd.h>

#include <stdint.h> /* EXtra types */

#include "fifo.h"

fifo *time_ticks;

static volatile int keepRunning = 1;



void intrHandler(int dummy) {
    keepRunning = 0;
}


uint64_t time_value, file_value;

int main(void) {
	
	
	struct timeval start, end;
		int count = 0; 
		
	FILE * pFile;
	
	  pFile = fopen ("myfile.txt","w");
	
	
	signal(SIGINT, intrHandler);
	
	time_ticks = new fifo;
	
	
	
	while (1) {
	
		 gettimeofday(&start, NULL);
	
		time_value = (start.tv_sec * (uint64_t)1000000) + (start.tv_usec);
		
		if (time_ticks->free()>0) 
		{
			printf("Free:%d\r\n", time_ticks->free() );
		
			time_ticks->put(time_value);
		}
		else
		{
			printf("No Room for the value, free: %d\r\n", time_ticks->free() );
			break;
		}
			
		
		
		count++;
		
		if (count > 1000) 
			break;
		
		
		
		// Sleep for 1 ms
		usleep(1000);
	
	}
	
	
	
	
	while (time_ticks->available())
	{
		time_ticks->get(&file_value);
		
		fprintf (pFile, "%ld\r\n", file_value);
	}
	
	
	fclose(pFile);
	
	delete time_ticks;
	
	return 0;
}
