/*
* Description: Demonstration of testing of Part 3 of the Project. 
*
* Author: Delaney Carleton
*
* Date: May 18, 2020
*
* Notes:
* 1. Demonstrate functionality of file commands for Part 3 of project
  2. This tests the functionality of reading the input command file 
     and then printing what's discussed on the project accordingly
     This reads the input command file given on Canvas in its testing

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
#define NUMPROXIES 6

/*---------------------------------------------------------------------------*/

//Based upon Linux Tutorial: POSIX Threads
//Add Mutex Init
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/*---------------------------------------------------------------------------*/
//Global Variables
int entry_number = 1;
int delta = 5;

//Global Variables to track topics, publishers, and subscribers
int index_count = 0;
int publisher_count = 0;
int subscriber_count = 0;

/*---------------------------------------------------------------------------*/
//Topic Entry struct 
//Based upon Project Description
typedef struct topicEntry
{
    int entryNum;
    struct timeval timeStamp;
    int pubID;
    char photoURL[URLSIZE];
    char photoCaption[CAPSIZE];

} topicEntry;

//Topic Entry Queue struct
//Based upon Project and Lab Description
typedef struct TEQ
{
    char *name;
    topicEntry *buffer;
    int head;
    int tail;
    int length;
    //Keep track of all entries
    int totalEntries;
    int topicID;

} TEQ;

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

//Based off of Professor Malony Code
void initialize() 
{
    //Define Variables
    int i;
    int j;
    int k;

    // create the buffers and initialize
    for (i=0; i<MAXTOPICS; i++) 
    {
        registry[i].length = 0;   // # entries in buffer now
        registry[i].head = 0;    // head index
        registry[i].tail = 0;    // tail index
        registry[i].buffer = (struct topicEntry *) malloc(sizeof(struct topicEntry) * MAXENTRIES);
        registry[i].totalEntries = 0; //total Entries
        registry[i].topicID = 0;
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

/*-------------------------- Free Queue Function ----------------------------------*/

//Function attempting to account for valgrind memory errors/leaks
//Free allocated buffer memory from initialize function
void free_queue()
{

    for(int i = 0; i < MAXTOPICS; i++)
    {
        free(registry[i].buffer);
    }

}

/*-------------------------- Enqueue Function ----------------------------------*/

//Based off Professor Malony Code
//Assistance from Professor Malony in Office Hours
//with understanding enqueue and how to get my function working properly
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
    //registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoURL = registry[TEQ_ID].photoURL;
    //registry[TEQ_ID].buffer[registry[TEQ_ID].tail].photoCaption = registry[TEQ_ID].photoCaption;

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
//Based off of cases described in Project Description and further Office Hour Discussion
//Canvas Discussion Board assisted in provided a clear description of each case and how to implement
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
//            printf("Registry not Empty-Else Statement Check\n");

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
                    strcpy(TE->photoURL, registry[i].buffer[j].photoURL);
                    strcpy(TE->photoCaption, registry[i].buffer[j].photoCaption);

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
                    strcpy(TE->photoURL, registry[i].buffer[j].photoURL);
                    strcpy(TE->photoCaption, registry[i].buffer[j].photoCaption);

                    //Return entryNum of this entry

                    return(registry[topicID].buffer[j].entryNum);
                }

        }
    }
        //Other Registry not empty and last entry + 1 < entryNum
        //Return 0
        printf("All Entries less than lastEntry+1\n");
        return 0;
}


/*----------------------------- Publisher Function -----------------------------------------*/

//Publisher Function 
//For Publisher Thread - use enqueue
//Professor Malony circular ring buffer and Office Hours assisted in understanding this
// implementation

void *publisher(void *args)
{
    //Define Variables
    //   pubpool[i].id = i;

    int thread_id;
    struct topicEntry entry;
    int interval = 0;

    thread_id = ((struct my_thread *) args)->id;

    printf("------------------------------------------------------\n");
    printf("Proxy Thread ID: %d, Type  - Publisher\n", thread_id);
    printf("------------------------------------------------------\n");

    //Run in Infinite Loop Check to see if publisher function properly
//    while(1)
 //  {
        //Loop through topics
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
                printf("Publisher ID %d Successful\n", thread_id);
                pthread_mutex_unlock(&(mutex[i]));
            }
        }

        //Interval increment used in testing for publisher functioning properly
        interval++;

/*
    Check to see if publisher is working properly
    //Check to see enqueue of topics after publisher sleeps and 
    //clean up threads continue to dequeue
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
        
  // }

    //Return
    return NULL;
}

/*------------------------------ Subscriber Function ----------------------------------------*/

//For Subscriber Thread - use getEntry
//Professor Malony circular ring buffer and Office Hours assisted in understanding this
// implementation
void *subscriber(void *args)
{

    //Define Variables
    int thread_id;
    struct topicEntry entry;
    int i;
    int return_value;

    thread_id = ((struct my_thread *) args)->id;
    
    printf("------------------------------------------------------\n");
    printf("Proxy Thread ID: %d . Type - Subscriber\n", thread_id);
    printf("------------------------------------------------------\n");

    //return NULL;

    //Initialize all entries
    for(int j = 0; j < MAXTOPICS; j++)
    {
        subpool[thread_id].lastEntries[j] = 1;
    }

//Infinite loop used to check functionality of subscriber and get entry function
//    while(1)
 //   {
    //Implement getEntry
    //Loop through topics
    for(i = 0; i < MAXTOPICS; i++)
    {

        pthread_mutex_lock(&(mutex[i]));
        
        //Get the return value from getEntry 
        return_value = getEntry(subpool[thread_id].lastEntries[i], &entry, i);

        //If Queue is Empty
        if(return_value == 0)
        {
            printf("Subscriber ID: %d Empty or Nothing New\n", thread_id); 

            pthread_mutex_unlock(&(mutex[i]));

        }

        else
        {
                //Entry in the queue
                if(return_value == 1)
                {
                    //Increment last entries
                    subpool[thread_id].lastEntries[i]++;
                }

                else
                {
                    //Otherwise entry greater than last entry + 1
                    //Set to return value
                    subpool[thread_id].lastEntries[i] = return_value;
                }
                 
                printf("Subscriber ID %d Successful at lastEntry %d\n", thread_id, subpool[thread_id].lastEntries[i]);
                printf("Entry Num %d - PubID: %d\n", entry.entryNum, entry.pubID);

                pthread_mutex_unlock(&(mutex[i]));
        }

  //  }

    //Schedule yield to check subscriber functionality - Assistance by Professor Malony in Office Hours
    sched_yield();

    }
    return NULL;
}

/*---------------------------------- Clean Up Thread Function --------------------------------------*/

//Clean Up Thread
//Based upon assistance in Professor Malony Office Hours 
//Dequeue function/implementation done within clean up thread function
//For Clean Up Thread - use dequeue
void *cleanup_thread(void *args)
{

    //Define Variables
    int i;

    struct timeval curtime;
    struct timeval oldtime;

    //Checks for delta and difference of time
   // int delta = 5;
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
//                printf("Old Time %d\n", oldtime);
//                printf("Curr Time %d\n", curtime);

                if( (diff_time) > delta)
                {

                    //Dequeue
                    printf("Dequeue Topic = %d. Entry Number %d, Length %d\n", i, 
                    registry[i].buffer[registry[i].head].entryNum, registry[i].length);

                    //Decrement length
                    registry[i].length--;
                    registry[i].head = (registry[i].head + 1 ) % MAXENTRIES;

                    //Unlock
                    pthread_mutex_unlock(&(mutex[i]));
                } 

                else 
                {
                    //Schedule Yield
                    //Discussed on Canvas Discussion board and office hours to include 
                    //Relinquish the CPU
                    sched_yield();
                }
            }
        }

        //Sleep to check when running
        //Sched yield to check running properly - Assitance from Professor Malony in Office Hours
       // sleep(1);
        sched_yield();
    }

    //Return
    return NULL;
} 

/*------------------------------ Parse Token Function ----------------------------------------*/

//Parse token function
//Based upon Project 1 implementations
//Used for tokenizing/Parsing input file
void parse_token(char *token)
{
    //Define Variables 
    char *parse_arg = NULL;
    char *args = NULL;

    char *args1 = NULL;
    char *args2 = NULL;
    char *args3 = NULL;
    char *args4 = NULL;
    parse_arg = strtok(token, " ");

    args1 = strtok(NULL, " \n");
    args2 = strtok(NULL, " \n");
    args3 = strtok(NULL, " \n");
    args4 = strtok(NULL, " \n");


    while(parse_arg != NULL)
    {

        if((strcmp(parse_arg, "create") == 0))
        {

 //          printf("args1 is: %s\n", args);

            if(args1 != NULL)
            {
                if((strcmp(args1, "topic") == 0))
                {
                    printf("\n");
                    printf("Creating a Topic\n");
                    printf("\n");

//                  printf("Arg2: %s\n", args2);
 //                 printf("Arg3: %s\n", args3);
  //                printf("Arg4: %s\n", args4);

                    //Create topic with name, id, and length to allocate
                    //Use atoi to set id and length to integers, based upon stackover discussion and office hours 
                    registry[index_count].name = strdup(args3);
                    registry[index_count].length = atoi(args4);
                    registry[index_count].topicID = atoi(args2);
    
                    //Increment count for topics
                    index_count++;
                }
            }
        }

        else if((strcmp(parse_arg, "query") == 0))
        {
            
            // printf("In Query, args1 is %s\n", args1);
                        
            //Query Topics
            if(args1 != NULL && (strcmp(args1,"topics") == 0))
            {
                
                printf("\n");
                printf("Query Topics\n");
                printf("--------------------------------------------------------------------\n");

                //Loop through number of topics that have been created
                for(int i = 0; i < index_count; i++)
                {
                    //Print out all topic IDs and their lengths
                    printf("Topic ID:  %d with Length: %d\n", registry[i].topicID, registry[i].length);
                }
                printf("-------------------------------------------------------------------\n");
             }
                
            //Query Publishers
            else if(args1 != NULL && (strcmp(args1,"publishers") == 0))
            {
                
                printf("Query Publishers \n");
                printf("-----------------------------------------------------------------\n");
                //Loop through number of publishers that have been created
                for(int i = 0; i < publisher_count; i++)
                {
                    //Print out current publishers and their command file names
                    printf("Publisher with Command File Name %s\n", pubpool[i].file_name); 
                }
                printf("-----------------------------------------------------------------\n"); 

            }

            //Query Subscribers
            else if(args1 != NULL && (strcmp(args1,"subscribers") == 0)) 
            {   
                printf("Query Subscribers\n");
                printf("-----------------------------------------------------------------\n"); 

                for(int i = 0; i < subscriber_count; i++)
                {   
                    //Print out current subscribers and their command file names
                    printf("Subscriber with Command File Name %s\n", subpool[i].file_name); 
                }   

                printf("-----------------------------------------------------------------\n"); 

            }  

        }

        //Add Publisher/Subscriber
        else if((strcmp(parse_arg, "add") == 0))
        {
//           printf("Add a Publisher or Subscriber\n");
 //          printf("Type is %s\n", args1);

            //Add a Publisher
            if(args1 != NULL && (strcmp(args1,"publisher") == 0))
            {
                if(args2 != NULL)
                {
                    //Loop through number of proxies in order to find available publisher in pool
                    //Once find an available publisher, set to unavailable (1) and break
                    //Based upon office hours assistance and discussion
                    for(int i = 0; i < NUMPROXIES; i++)
                    {
                        if(pubpool[i].available == 0)
                        {
                            printf("\n");
                            printf("Publisher Thread Added\n");
                            printf("\n");
                            //Set to unavailable
                            pubpool[i].available = 1;
                            //Copy file name into publisher
                            strcpy(pubpool[i].file_name, args2);
                            //Increment number of publishers
                            publisher_count++;
                            //Break from loop once found available publisher
                            break;
                        }
                    }
                }

            }

            //Add a Subscriber
            else if(args1 != NULL && (strcmp(args1, "subscriber") == 0))
            {

                if(args2 != NULL)
                {   
                    //Loop through numproxis to find available subscriber in pool
                    //Once find an available subscriber, set to unavailable (1) and break
                    //Based upon office hours with Professor Malony
                    for(int i = 0; i < NUMPROXIES; i++)
                    {   
                        if(subpool[i].available == 0)
                        {   
                            printf("\n");
                            printf("Subscriber Thread Added\n");
                            printf("\n");
                            //Set to unavailable
                            subpool[i].available = 1;
                            //Copy file name into subscriber
                            strcpy(subpool[i].file_name, args2);
                            //Increment number of subscribers
                            subscriber_count++;
                            //Break from loop once found available subscriber
                            break;
                        }   
                    }   
                }   

            }
        
        }   

        //Set delta to specified value
        else if ((strcmp(parse_arg, "delta") == 0)) 
        {   

            if(args1 != NULL)
            {
                //Set delta to specified value - delta is a global variable
                //Use atoi to change arg to integer, based on stackoverflow discussion
                delta = atoi(args1); 
                printf("\n");
                printf("Delta Set to: %d\n", atoi(args1));
                printf("\n");
            }
        }   

        //Start all publishers and subscribers
        else if ((strcmp(parse_arg, "start") == 0)) 
        {   
            if(args1 == NULL)
            {
                printf("\n");
                printf("Starting All Publishers and Subscribers\n");
                printf("\n");

               // printf("Breaking Out of Loop from Start\n");

                //Break from loop to go into creating the publisher and subscriber threads within main
                break;
            }
        }   

       //Otherwise, Invalid Command 
        else
        {   
            printf("Error! Unrecognized Command: %s\n", parse_arg);
        }   

        //Parse argument with " "
        parse_arg = strtok(NULL, " ");

    }
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

    char *buffer;
    char *token = NULL;
    FILE *stream;
    ssize_t nread;
    size_t bufsize = 1024;
    char *save_buffer = NULL;
    int condition = 1;
    char *parse_arg = NULL;
    char *arg1 = NULL;
    const char delim[1] = " ";

    //Call Initializer
    initialize();

    //Check for File
    if(argc < 2)
    {

        printf("Error! No File Given\n");
    }

    else if (argc > 2)
    {
        printf("Error! Too many arguments\n");

    }

    else if (argc == 2)
    {
//        printf("Opening File\n");
        stream = fopen(argv[1], "r");

        //Check for Valid File
        if(stream == NULL)
        {

            printf("Error! Unable to Open File. Exiting\n");
            //Exit
            exit(1);
        }

        //Read each line of input file
        while(((nread=getline(&buffer, &bufsize,stream)) != -1))
        {

            //Save buffer used for strtok_r
            save_buffer = buffer;

            //Tokenize usimg implementation in Project 1 through using strtok_r and
            //a save_buffer. Tokenize on newline and carriage return
            while((token = strtok_r(save_buffer, ";\n\r", &save_buffer)) != NULL)
            { 

                //Call parse token function to tokenize commands
                parse_token(token);

                parse_arg = strtok(token, " ");
  //              arg1 = strtok(NULL, " \n");

//                printf("Parse Arg %s\n", parse_arg);

            }
        }
    }

    //Create the publisher and subscriber threads
    for(i = 0; i < NUMPROXIES; i++)
    {
            //Check for Publisher
            if(pubpool[i].available == 1)
            {
//                printf("Publisher Starting\n");
                args[i].id = i;

                pthread_attr_init(&pubpool[i].attr);
                pthread_create(&pubpool[i].this_thread, &pubpool[i].attr, publisher, (void *) &args[i]);

            }

            //Check for Subscriber
            if(subpool[i].available == 1)
            {
 //               printf("Subscriber Starting\n");
                args[i].id = i;
                pthread_attr_init(&subpool[i].attr);
                pthread_create(&subpool[i].this_thread, &subpool[i].attr, subscriber, (void *) &args[i]);
                
            }

    }

    //Create Clean Up Thread
    pthread_attr_init(&cleanattr);
    pthread_create(&clean_thread, &cleanattr, cleanup_thread, (void *) &cleanarg);

    //Set the publishers back to available once they have been created
    //As discussed on the Canvas Discussion board and office hours with Professor Malony and Jared, 
    //set publishers back to available and use pthread_join for publishers to finish
    for(int i = 0; i < NUMPROXIES; i++)
    {
        if(pubpool[i].available == 1)
        {
            //Use pthread_join to wait for termination as discussion on Canvas Discussion board
            pthread_join(pubpool[i].this_thread,NULL);
        }

        //Set back to available
        pubpool[i].available = 0;

    }

    //Set the subscribers back to available once they have been created
    //As discussed on Canvas Discussion Board and office hours, 
    //set publishers back to available and use pthread_join for subscribers to finish
    for(int i = 0; i < NUMPROXIES; i++)
    {

        if(subpool[i].available == 1)
        {   
            //Use pthead_join to wait for termination as discussion on Canvas Discussion board
            pthread_join(subpool[i].this_thread, NULL);
        }   

        //Set back to available
        subpool[i].available = 0;
    }
    
/*
    //Create Publisher Threads
    for(i = 0; i < NUMPROXIES; i++) 
    {

    //  pubpool[i].id = i;
        args[i].id = i;

        //Create Publishers
        pubpool[i].available = 1;
        pthread_attr_init(&pubpool[i].attr);
//        pthread_create(&pubpool[i].this_thread, &pubpool[i].attr, publisher, (void *) &args[i]);

        //Create Subscribers

        subpool[i].available = 1;
        pthread_attr_init(&subpool[i].attr);
 //       pthread_create(&subpool[i].this_thread, &subpool[i].attr, subscriber, (void *) &args[i]);

    }

    //Create Clean Up Thread
    pthread_attr_init(&cleanattr);
  //  pthread_create(&clean_thread, &cleanattr, cleanup_thread, (void *) &cleanarg);

*/
    //Sleep to check Running
//    sleep(5);
//    while(1)
 //   {
  //      sched_yield();
   // }


    //Close File
    fclose(stream); 

    //Free Allocated Memory
    free(buffer);

    //Free allocated memory from initialize function
    free_queue();

    //Exit Program
    printf("Program Exiting\n");

    //Return
    return 0;


}
