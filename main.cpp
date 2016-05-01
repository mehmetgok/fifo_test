#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h> /* Ctrl+C Handling */
#include <unistd.h>
#include <stdint.h> /* EXtra types */

#include "fifo.h"


fifo *fifoData, *fifoProdTimes, *fifoConsTimes;

static volatile int keepRunning = 1;

void intrHandler(int dummy) {
    keepRunning = 0;
}



pthread_t thDataGetter, thDataProcessor;

void dataGetter();
void dataProcessor();



int main(void) {
	
	
	FILE * pFileProdTimes, *pFileConsTimes;
	
	pFileProdTimes = fopen ("prodTimes.txt","w");
	pFileConsTimes = fopen ("prodTimes.txt","w");
	
	
	signal(SIGINT, intrHandler);
	
	fifoData = new fifo;
	
	fifoProdTimes = new fifo;
	fifoConsTimes = new fifo;
	
	
	pthread_create(&thDataGetter, NULL, (void *) &dataGetter, NULL);
	pthread_create(&thDataProcessor, NULL, (void *) &dataProcessor, NULL);
	
	
	
	
	
	
	delete fifoConsTimes;
	delete fifoProdTimes;
	delete fifoData;
	
	
	fclose(pFileProdTimes);
	fclose(pFileConsTimes);
	
	
	return 0;
}

void dataGetter() {
	
	
	while (keepRunning) {
		
		
		
		usleep(5000);
	}
	
}

void dataProcessor() {
	
	while (keepRunning) {
		
		
		
		
		usleep(5000);
	
	}
}
