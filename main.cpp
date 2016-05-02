#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h> /* Ctrl+C Handling */
#include <unistd.h>
#include <stdint.h> /* Extra types */

#include "fifo.h"

#define STEP_LIMIT 100

fifo *fifoData, *fifoProdTimes, *fifoConsTimes;

static volatile int keepRunning = 1;

void intrHandler(int dummy) {
    keepRunning = 0;
}

pthread_t thDataGetter, thDataProcessor;

pthread_mutex_t lock;

void *dataGetter(void*);
void *dataProcessor(void*);


int main(void) {
	
	
	uint64_t file_value;
	
	
	FILE * pFileProdTimes, *pFileConsTimes;
	
	pFileProdTimes = fopen ("prodTimes.txt","w");
	pFileConsTimes = fopen ("consTimes.txt","w");
	
	
	signal(SIGINT, intrHandler);
	
	fifoData = new fifo;
	
	fifoProdTimes = new fifo;
	fifoConsTimes = new fifo;
	
	
	pthread_create(&thDataGetter, NULL, &dataGetter, NULL);
	pthread_create(&thDataProcessor, NULL,  &dataProcessor, NULL);
		
	
	pthread_join( thDataGetter, NULL);	
	pthread_join( thDataProcessor, NULL);
	
	while (fifoProdTimes->available())
	{
		fifoProdTimes->get(&file_value);
		
		fprintf (pFileProdTimes, "%lld\r\n", file_value);
	}
	
	while (fifoConsTimes->available())
	{
		fifoConsTimes->get(&file_value);
		
		fprintf (pFileConsTimes, "%lld\r\n", file_value);
	}
	
	
	delete fifoConsTimes;
	delete fifoProdTimes;
	delete fifoData;
	
	
	fclose(pFileProdTimes);
	fclose(pFileConsTimes);
	
	
	return 0;
}

void *dataGetter(void*) {
	
	int counts = 1;
	struct timeval prodTime;
	
	uint64_t prod_time;

	
	
	while (keepRunning) {
		
		
		// Add value to the queue
		
		
		pthread_mutex_lock(&lock);
		if (fifoData->free()>0) 
		{
			// printf("Free:%d\r\n", fifoData->free() );
		
			fifoData->put((uint64_t) counts);
			
			
				gettimeofday(&prodTime, NULL);
		
		prod_time = (prodTime.tv_sec * (uint64_t)1000000) + (prodTime.tv_usec);
		
		if (fifoProdTimes->free()>0) 
		{
			// printf("Free:%d\r\n", fifoProdTimes->free() );
		
			fifoProdTimes->put(prod_time);
		}
		else
		{
			printf("No Room for the value, free: %d\r\n", fifoProdTimes->free() );
			break;
		}
			
				
		counts++;
			
			
		}
		else
		{
			printf("No Room for the value, free: %d\r\n", fifoData->free() );
			break;
		}
		
		
		pthread_mutex_unlock(&lock);
		
		usleep(500);
	
		
		if (counts>STEP_LIMIT)
			break;
	}
	
}

void *dataProcessor(void*) {
	
	
	int counts = 1;
	struct timeval consTime;
	
	uint64_t cons_time;
	
	// Data taken from dataFifo
	uint64_t count_data;
	
	while (keepRunning) {
		
		
		// Get data from queue
		pthread_mutex_lock(&lock);
		if (fifoData->available()>0) 
		{
			// printf("Free:%d\r\n", fifoData->free() );
		
			fifoData->get(&count_data);
			
			
				gettimeofday(&consTime, NULL);
		
		cons_time = (consTime.tv_sec * (uint64_t)1000000) + (consTime.tv_usec);
		
		if (fifoConsTimes->free()>0) 
		{
			// printf("Free:%d\r\n", fifoProdTimes->free() );
		
			fifoConsTimes->put(cons_time);
		}
		else
		{
			printf("No Room for the value, free: %d\r\n", fifoConsTimes->free() );
			
		}
		
		
		
		
		
		
		counts++;
			
			
			
			
			
			
			
			
			
			
			
			
		}
		else
		{
			printf("No Data available int the queue: %d\r\n",  fifoData->available() );
		
		}
		
		
    	 pthread_mutex_unlock(&lock);
		 
		 usleep(500);
		
	
		
		if (counts>STEP_LIMIT)
			break;
	
	}
}
