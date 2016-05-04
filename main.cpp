#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h> /* Ctrl+C Handling */
#include <unistd.h>
#include <stdint.h> /* Extra types */


/* Linux RT */
#include <sched.h>
#include <sys/mman.h>
#include <string.h>

//#include <linux/spinlock.h>


#include "fifo.h"

#define STEP_LIMIT 1000


#define BILLION  1000000000L

fifo *fifoData, *fifoProdTimes, *fifoConsTimes;

static volatile int keepRunning = 1;

void intrHandler(int dummy) {
    keepRunning = 0;
}


#define MY_PRIORITY_1 (49) /* we use 49 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */
                            
                            
#define MY_PRIORITY_2 (45) /* we use 48 as the PRREMPT_RT use 50
                            as the priority of kernel tasklets
                            and interrupt handler by default */

#define MAX_SAFE_STACK (8*1024) /* The maximum stack size which is
                                   guaranteed safe to access without
                                   faulting */

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

void stack_prefault(void) {

        unsigned char dummy[MAX_SAFE_STACK];

        memset(dummy, 0, MAX_SAFE_STACK);
        return;
}




pthread_t thDataGetter, thDataProcessor;

pthread_mutex_t lock;


void *dataGetter(void*);
void *dataProcessor(void*);


int main(void) {
	
	
	uint64_t file_value1, file_value2;
	
	
	FILE * pFileProdTimes, *pFileConsTimes, *pFileDifTimes;
	
	pFileProdTimes = fopen ("prodTimes.txt","w");
	pFileConsTimes = fopen ("consTimes.txt","w");
	pFileDifTimes = fopen ("difTimes.txt","w");
		
	
	 /* Lock memory */

        if(mlockall(MCL_CURRENT|MCL_FUTURE) == -1) {
            perror("mlockall failed");
            exit(-2);
        }
		
			stack_prefault();
	
	signal(SIGINT, intrHandler);
	
	// printf("Size of Int: %d", sizeof(unsigned long long));
	
	fifoData = new fifo;
	
	fifoProdTimes = new fifo;
	fifoConsTimes = new fifo;
	
	
	if (pthread_mutex_init(&lock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }
	
	
	pthread_mutex_lock(&lock);

	pthread_create(&thDataProcessor, NULL,  &dataProcessor, NULL);
	pthread_create(&thDataGetter, NULL, &dataGetter, NULL);
	
		
    pthread_mutex_unlock(&lock);
		
	
	pthread_join( thDataGetter, NULL);	
	pthread_join( thDataProcessor, NULL);
	
	
	pthread_detach(thDataGetter);
	pthread_detach(thDataProcessor);
	 
	pthread_mutex_destroy(&lock);


	
	while (fifoProdTimes->available())
	{
		fifoProdTimes->get(&file_value1);
		fifoConsTimes->get(&file_value2);
		
		fprintf (pFileProdTimes, "%llu\r\n", file_value1);
		fprintf (pFileConsTimes, "%llu\r\n", file_value2);
		fprintf (pFileDifTimes, "%llu\r\n", file_value2-file_value1);
	}
	
	/*while (fifoConsTimes->available())
	{
		
		
		fprintf (pFileConsTimes, "%llu\r\n", file_value);
	}*/
	
	
	delete fifoConsTimes;
	delete fifoProdTimes;
	delete fifoData;
	
	
	fclose(pFileProdTimes);
	fclose(pFileConsTimes);
	fclose(pFileDifTimes);
	
	

	
	
	
	
	return 0;
}

void *dataGetter(void*) {
	
	int counts = 1;
	// struct timeval prodTime;
	
	struct timespec prodTime;
	
	struct sched_param param;
	
	struct timespec t;
	int interval = 900000; /* 500 us*/
	
	uint64_t prod_time;
	
	
	
	/* Declare ourself as a real time task */

        param.sched_priority = MY_PRIORITY_1;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler failed");
                exit(-1);
        }
		
        
           clock_gettime(CLOCK_MONOTONIC ,&t);
        /* start after 500 usecs */
          t.tv_nsec += interval;

	
	
	while (keepRunning) {
		
		      /* wait until next shot */
                clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
				
				
		
		// Add value to the queue
		
			pthread_mutex_lock(&lock);
	

		if (fifoData->free()>0) 
		{
			// printf("Free:%d\r\n", fifoData->free() );
		
			fifoData->put((uint64_t) counts);
			
			
			// gettimeofday(&prodTime, NULL);
		
			// prod_time = (prodTime.tv_sec * (uint64_t)1000000.0) + (uint64_t)(prodTime.tv_usec);
			
			
			// Get real time clock
			clock_gettime( CLOCK_REALTIME, &prodTime);	
			
			// convert to micro seconds			
			prod_time = (prodTime.tv_sec*(uint64_t)BILLION + (uint64_t)prodTime.tv_nsec)/1000;
			
			
		
			if (fifoProdTimes->free()>0) 
			{
				// printf("Free:%d\r\n", fifoProdTimes->free() );
			
				fifoProdTimes->put((uint64_t)prod_time);
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
		

	//	usleep(500);
	
	
		 /* calculate next shot */
                t.tv_nsec += interval;

                while (t.tv_nsec >= NSEC_PER_SEC) {
                       t.tv_nsec -= NSEC_PER_SEC;
                        t.tv_sec++;
                }
		
	
		
		if (counts>STEP_LIMIT)
			break;
	}
	
}

void *dataProcessor(void*) {
	
	
	int counts = 1;
	// struct timeval consTime;
	
	struct timespec consTime;
	
	struct sched_param param;
	struct timespec t;
	int interval = 700000; /* 400 us*/
	
	uint64_t cons_time;
	
	// Data taken from dataFifo
	uint64_t count_data;
	
	
	

	
	
	/* Declare ourself as a real time task */

        param.sched_priority = MY_PRIORITY_2;
        if(sched_setscheduler(0, SCHED_FIFO, &param) == -1) {
                perror("sched_setscheduler failed");
                exit(-1);
        }
		
	

      
	
	   clock_gettime(CLOCK_MONOTONIC ,&t);
        /* start after 500 usecs */
          t.tv_nsec += interval;
        
	
	
	while (keepRunning) {
		
		 clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		 
		
		
		pthread_mutex_lock(&lock);
			
		

		if (fifoData->available()>0) 
		{
			// printf("Free:%d\r\n", fifoData->free() );
			fifoData->get(&count_data);
			// gettimeofday(&consTime, NULL);
			//cons_time = (consTime.tv_sec * (uint64_t)1000000) + (uint64_t)(consTime.tv_usec);
			
			
			// Get realtime clock
			clock_gettime( CLOCK_REALTIME, &consTime);			
			
			// convert to micro seconds			
			cons_time = (consTime.tv_sec*(uint64_t)BILLION + (uint64_t)consTime.tv_nsec)/1000;
			
			
		
			if (fifoConsTimes->free()>0) 
			{
				// printf("Free:%d\r\n", fifoProdTimes->free() );
			
				fifoConsTimes->put((uint64_t)cons_time);
			}
			else
			{
				printf("No Room for the value, free: %d\r\n", fifoConsTimes->free() );
				
			}
				
		
			counts++;		
			
			
		}
		else
		{
			// printf("No Data available int the queue: %d\r\n",  fifoData->available() );		
		}
		
		pthread_mutex_unlock(&lock);
    	
		 
		// usleep(500);
		
		 /* calculate next shot */
                t.tv_nsec += interval;

                while (t.tv_nsec >= NSEC_PER_SEC) {
                       t.tv_nsec -= NSEC_PER_SEC;
                        t.tv_sec++;
                }
		
	
		
		if (counts>STEP_LIMIT)
			break;
	
	}
}
