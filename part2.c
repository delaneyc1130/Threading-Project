/*
* Description: Demonstration of testing of Part 2 of the project to show functionality of
               enqueu, dequeue, getEntry with the publisher/subscriber proxy threads

* Author: Delaney Carleton
*
* Date: May 18,2020
*
* Notes:
* 1. Demonstration of Part 1/2 of the project to show functionality
     of Enqueue, Dequeue, GetEntry
  2. In order to check the functionality of the functions and the proxy
      threads, I have my tests run in an infinite loop in order to check 
      the the enqueue, dequeue, getEntry functionalities are working.
      Thus my publisher in enqueueing infinitely in order to check my 
      enqueu function as well as my dequeue in the topic clean up thread
      and then getEntry within the publisher.
     The program then sleeps for 10 seconds to show this functionality and
     then exits
*/

/*-------------------------Preprocessor Directives---------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <semaphore.h>

#define MAXENTRIES 20
#define MAXNAME 10
#define MAXTOPICS 5
#define URLSIZE 50
#define CAPSIZE 50
#define MAXSIZE 100
#define NUMPROXIES 3


/*---------------------------------------------------------------------------*/
//Based upon Linux Tutorial: POSIX Threads
//Add Mutex Init
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*---------------------------------------------------------------------------*/
//Global Variables
int entry_number = 1;
static int running = 0;

/*---------------------------------------------------------------------------*/
typedef struct topicEntry
{
    int entryNum;
    struct timeval timeStamp;
    int pubID;
    char photoURL[URLSIZE];
    char photoCaption[CAPSIZE];

} topicEntry;

/*---------------------------------------------------------------------------*/
typedef struct TEQ
{
    char *name[MAXNAME];
    topicEntry *buffer;
    int head;
    int tail;
    int length;
    //Keep track of all entries
    int totalEntries;
    int topicID;

} TEQ;

/*---------------------------------------------------------------------------*/

//Struct for Publisher and Subscriber
//Described by Jared in Office Hours
typedef struct my_thread
{
    int id;
    int alive;
    int available;
    int lastEntries[MAXTOPICS];
    char file_name[100];
    int pubID;

    pthread_t this_thread;
    pthread_attr_t  attr;   

} my_thread;

//Based off Professor Malony code 
struct threadargs 
{
  int   id;
};


//Registry for TEQ
TEQ registry[MAXTOPICS];

//Mutex
pthread_mutex_t mutex[MAXTOPICS];

//Pools for Threads for Publisher and Subscriber
my_thread pubpool[NUMPROXIES];
my_thread subpool[NUMPROXIES];

/*------------------------ Initializer Function ---------------------------------*/
//Initializer Function
//Based off of Professor Malony Code
void initialize() 
{
    int i, j, k;

    // create the buffers
    for (i=0; i<MAXTOPICS; i++) 
    {
        registry[i].length = 0;   // # entries in buffer now
        registry[i].head = 0;    // head index
        registry[i].tail = 0;    // tail index
        registry[i].buffer = (struct topicEntry *) malloc(sizeof(struct topicEntry) * MAXENTRIES);
        registry[i].totalEntries = 0; //total Entries
    }

    //Initialize mutex 
    for (i=0; i<MAXTOPICS; i++) 
    {
        pthread_mutex_init(&(mutex[i]), NULL);
    }

    //Initialize Pub/Sub to Available
    for(i = 0; i<NUMPROXIES; i++)
    {
        pubpool[i].available = 0;
        subpool[i].available = 0;
    }

} 

/*-------------------------- Enqueue Function ----------------------------------*/
//Enqueue Function
//Based off Professor Malony Code
int enqueue(int TEQ_ID, topicEntry *TE)
{
    //Define Variables
    int i;
    struct timeval tv;

    //Check if Buffer Full
    if (registry[TEQ_ID].length == MAXENTRIES) 
    {
        //Return -1
        return(-1);     // buffer is full
    }

    //Increment length
    registry[TEQ_ID].length++;

    //Increment totalEntries
    registry[TEQ_ID].totalEntries++;

    //Index into TEQ at tail, select field - and set to totalEntries
    //Same thing for everything in struct
    registry[TEQ_ID].buffer[registry[TEQ_ID].tail].entryNum = registry[TEQ_ID].totalEntries;
  
    registry[TEQ_ID].buffer[registry[TEQ_ID].tail].pubID = registry[TEQ_ID].topicID;
//    registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoURL = registry[TEQ_ID].photoURL;
//    registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoCaption = registry[TEQ_ID].photoCaption;
    strcpy(registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoCaption, TE->photoCaption);
    strcpy(registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoURL, TE->photoURL);

    //Put in TimeStamp
   // gettimeofday(&tv, NULL);
   // printf("Time in Enqueue is: %d\n",tv);

    //Store same way as entryNum
    //registry[TEQ_ID].buffer[registry[TEQ_ID].tail].timeStamp = tv;
    gettimeofday(&registry[TEQ_ID].buffer[registry[TEQ_ID].tail].timeStamp, NULL);

//    printf("Time in Enqueue is: %d\n", registry[TEQ_ID].buffer[registry[TEQ_ID].tail].timeStamp);
    //registry[TEQ_ID].buffer[registry[TEQ_ID].tail].timeStamp = registry[TEQ_ID].totalEntries;

    registry[TEQ_ID].tail = (registry[TEQ_ID].tail + 1) % MAXENTRIES;

    //Return 0
    return 0;   
}

/*------------------------------Get Entry Function -------------------------------------------*/
//Get Entry function - Used in Subscriber
int getEntry(int lastEntry, topicEntry *TE, int topicID)
{
    //Define Variables
    int i;
    int j = 0;
    int ret_value;


//    printf("In GetEntry Function\n");

    //Loop through topic queue to search for lastEntry
 //       printf("Looping through Topic Queue to Search for lastEntry\n");

        //If it's empty
        if (registry[topicID].length == 0)  
        {   
            printf("lastEntry - Registry is Empty\n");
            return 0;
        }  


        else
        {
            printf("Registry not Empty-Else Statement Check\n");

            //Start at tail of queue
            for(j = registry[topicID].tail; j < registry[topicID].head; j++)
            {
                //Check if lastEntry+1 in Queue
                if(registry[topicID].buffer[j].entryNum == (lastEntry + 1))
                {
                
                    printf("Entry in the Queue\n");

                    //Copy entry into struct 
                    TE->entryNum = registry[topicID].buffer[j].entryNum;
                    TE->timeStamp = registry[topicID].buffer[j].timeStamp;
                    TE->pubID = registry[topicID].buffer[j].pubID;

                    //Since char have to use strcpy
                    //strcpy(TE->photoURL, registry[i].buffer[j].photoURL);
                    //strcpy(TE->photoCaption, registry[i].buffer[j].photoCaption);

                    //Return 1

                    return 1;
                }
            }

            //Case 3: Registry not Empty and Last Entry + 1 Not in Queue
            for(j = registry[topicID].tail; j < registry[topicID].head; j++)
            {

                //If entry greater than lastEntry + 1
                //Copy entry to empty topicEntry struct
                //Return entryNum of that entry
                if(registry[topicID].buffer[j].entryNum > (lastEntry + 1))
                {

                    printf("Entry Greater than lastEntry + 1\n");
                    //Copy Entry into topicEntry struct

                    TE->entryNum = registry[topicID].buffer[j].entryNum;
                    TE->timeStamp = registry[topicID].buffer[j].timeStamp;
                    TE->pubID = registry[topicID].buffer[j].pubID;
                    //strcpy(TE->photoURL, registry[i].buffer[j].photoURL);
                    //strcpy(TE->photoCaption, registry[i].buffer[j].photoCaption);

                    //Return entryNum of this entry

                    return(registry[topicID].buffer[j].entryNum);
                }

        }
    }
        //Other Registry not empty and last entry + 1 < entryNum
        //Return 0
        printf("All Entriess less than lastEntry+1\n");
        return 0;
}


/*----------------------------- Publisher Function -----------------------------------------*/
//Publisher Function 
//For Publisher Thread - use enqueue
void *publisher(void *args)
{
    //Define Variables
    //   pubpool[i].id = i;


    int thread_id;
    struct topicEntry entry;
    int interval = 0;

    running--;

    thread_id = ((struct my_thread *) args)->id;

    printf("Proxy Thread ID: %d,  - Publisher\n", thread_id);

    //Run in Infinite Loop Check
    while(1)
   {
        for(int i = 0; i < MAXTOPICS; i++)
        {

            pthread_mutex_lock(&(mutex[i]));

            //If Queue is Full- Can't Enqueue
            if (enqueue(i, &entry) == -1) 
            {
                printf("Publisher ID %d Full\n", thread_id);

                pthread_mutex_unlock(&(mutex[i]));

            }

            //Otherwise Success
            else 
            {
                printf("Publisher ID %d Successful Enqueue\n", thread_id);
                pthread_mutex_unlock(&(mutex[i]));
            }
        }

        interval++;

/*
    Check to see if publisher is working properly
        if(interval < 50)
        {
            sched_yield();
        }
        else
        {

            sleep(5);
            interval = 0;
        }
*/

        
   }

    //Return
    return NULL;
}

/*------------------------------ Subscriber Function ----------------------------------------*/
//Subscriber Function
//For Subscriber Thread - use getEntry
void *subscriber(void *args)
{

    //Define Variables
    int thread_id;
    struct topicEntry entry;
    int i;
    int return_value;


    thread_id = ((struct my_thread *) args)->id;
    
    printf("Proxy Thread ID: %d - Subscriber\n", thread_id);

    //return NULL;

    for(int j = 0; j < MAXTOPICS; j++)
    {
        subpool[thread_id].lastEntries[j] = 1;
    }

    while(1)
    {
    //Implement getEntry
    for(i = 0; i < MAXTOPICS; i++)
    {


        pthread_mutex_lock(&(mutex[i]));
        
        return_value = getEntry(subpool[thread_id].lastEntries[i], &entry, i);

        //If Queue is Empty
        if(return_value == 0)
        {
            printf("Subscriber ID: %d Empty or Nothing New\n", thread_id); 

            pthread_mutex_unlock(&(mutex[i]));

        }

        else
        {
                if(return_value == 1)
                {
                    subpool[thread_id].lastEntries[i]++;
                }

                else
                {
                    subpool[thread_id].lastEntries[i] = return_value;
                }
                 
                printf("Subscriber ID %d Successful at lastEntry %d\n", thread_id, subpool[thread_id].lastEntries[i]);
                printf("Entry Num %d - PubID: %d\n", entry.entryNum, entry.pubID);
                pthread_mutex_unlock(&(mutex[i]));
        }

    }

    sched_yield();

    }
    return NULL;
}

/*---------------------------------- Clean Up Thread Function --------------------------------------*/
//Clean Up Thread
//For Clean Up Thread - use dequeue
void *cleanup_thread(void *args)
{

    //Define Variables
    int i;

    struct timeval curtime;
    struct timeval oldtime;

    int delta = 5;
//    int diff_time = 7;
    int diff_time;

    //Get Current Time of Day
   // gettimeofday(&curtime, NULL);

    //Run in Infinite Loop
    while(1) {

        //Loop through all 
        for (i =0; i < MAXTOPICS; i++) 
        {


            pthread_mutex_lock(&(mutex[i]));

            //If it's empty
            if (registry[i].length == 0) 
            {
                pthread_mutex_unlock(&(mutex[i]));
                printf("Clean Up Thread Detect Empty at %d\n", i);
            }

            else 
            {

                //Calculate Time Difference
                //oldtime = registry[i].buffer[i].timeStamp;

                //diff_time = ((int) (curtime.tv_sec)) - ((int) (oldtime.tv_sec));
                //printf("Diff Time: %d\n", diff_time);
                gettimeofday(&curtime, NULL);
                oldtime = registry[i].buffer[registry[i].tail].timeStamp;
                diff_time = difftime(curtime.tv_sec, oldtime.tv_sec);
//                printf("Diff Time %d\n", diff_time);
 //               printf("Old Time %d\n", oldtime);
  //              printf("Curr Time %d\n", curtime);

                if( ((int)curtime.tv_sec) > delta)
                {

                    //Dequeue
                    printf("Dequeue Topic = %d. Entry Number %d, Length %d\n", i, 
                    registry[i].buffer[registry[i].head].entryNum, registry[i].length);

                    registry[i].length--;
                    registry[i].head = (registry[i].head + 1 ) % MAXENTRIES;

                    //Unlock
                    pthread_mutex_unlock(&(mutex[i]));

                } 
                else 
                {
                    //Scheduler Yield
                    sched_yield();
                }
            }
        }

        //Sleep to check when running
       // sleep(1);
        sched_yield();
    }

    //Return
    return NULL;
} 

/*---------------------------------------Main Program-------------------------------------------------------------- */
int main(int argc, char *argv[])
{

    //Define Variables
    int i;
    struct threadargs args[NUMPROXIES];
    struct threadargs cleanarg;
    pthread_attr_t  cleanattr;
    pthread_t clean_thread;

    //Call Initializer
    initialize();
    
    //Create Publisher Threads
    for(i = 0; i < NUMPROXIES; i++) 
    {

    //  pubpool[i].id = i;
        args[i].id = i;

        //Create Publishers
        pubpool[i].available = 1;
        pthread_attr_init(&pubpool[i].attr);
        pthread_create(&pubpool[i].this_thread, &pubpool[i].attr, publisher, (void *) &args[i]);

        //Create Subscribers

        subpool[i].available = 1;
        pthread_attr_init(&subpool[i].attr);
        pthread_create(&subpool[i].this_thread, &subpool[i].attr, subscriber, (void *) &args[i]);

    }

    //Create Clean Up Thread
    pthread_attr_init(&cleanattr);
    pthread_create(&clean_thread, &cleanattr, cleanup_thread, (void *) &cleanarg);


/*
  // Create the Subscriber Threads
  for(i = 0; i < NUMPROXIES; i++) {
//    subpool[i].id = i;
    args[i].id = i;
    pthread_attr_init(&subpool[i].attr);
    pthread_create(&subpool[i].this_thread, &subpool[i].attr, subscriber, (void *) &args[i]);
  }
*/
    //Sleep to check Running
    sleep(5);
//    while(1)
 //   {
  //      sched_yield();
   // }


    printf("Program Exiting\n");

    //Return
    return 0;


}
