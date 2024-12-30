/*******************************************************************************
* File Name          : circularQueueV2023V1.cpp_Assignment3
* Description        : circular Queue (Enqueue & Dequeue Operation)
*                      
*
* Author:            : Mohamed Yusuf
* Date:              : November 26 2025
******************************************************************************/
#include <windows.h>
#include <stdio.h>

DWORD WINAPI myReadThreadFunction(LPVOID lpParam);                                   // function used by read from queue thread
unsigned int putToCircularQueue(char* ptrInputBuffer, unsigned int bufferLength);    // circular queue function to add data to queue
unsigned int getFromCircularQueue(char* ptrOutputBuffer, unsigned int bufferLength); // circular queue function to remove data from queue

#define BUFFER_SIZE         200                // local buffer used for sending or receiving data to/from queue in main and worker thread
#define INPUT_TIMEOUT_MS    5000               // dequeue every 5 seconds
#define CIRCULAR_QUEUE_SIZE 10                 // size of the circular queue

CRITICAL_SECTION cs; // define a critical section variable

// data structure used to keep track of circular queue
struct myQueueStruct {
    char* ptrBuffer = NULL;                    // pointer to start of the circular queue buffer
    volatile unsigned int queueCount = 0;      // number of characters currently in circular queue
    volatile char* ptrCircularHead = NULL;     // location where data is added to queue
    volatile char* ptrCircularTail = NULL;     // location where data is removed from queue
};

struct myQueueStruct myQueue;                  // create an instance of the circular queue data structure

int main()
{
    HANDLE  hThread;            // Handle to the thread
    char    inputBuffer[BUFFER_SIZE]; // Buffer to store input data from the user

    // Allocate memory for the circular queue buffer
    myQueue.ptrBuffer = (char*)calloc(CIRCULAR_QUEUE_SIZE, sizeof(char));
    if (myQueue.ptrBuffer == NULL) // Check if memory allocation was successful
    {
        printf("Error: Unable to allocate memory for buffer\n");
        exit(-1); // Exit the program in case of failure
    }

    myQueue.ptrCircularHead = myQueue.ptrCircularTail = myQueue.ptrBuffer;

    InitializeCriticalSection(&cs); // initialize the critical section

    // create a thread that will consume the data we type in to demonstrate dequeuing the data
    hThread = CreateThread(
        NULL,                   // default security attributes
        0,                      // use default stack size  
        myReadThreadFunction,   // thread function name
        NULL,                   // argument to thread function (not being used)
        0,                      // use default creation flags 
        NULL);                  // returns the thread identifier (not being used)

    if (hThread != NULL) {      // ensure the creation of thread was successful
        printf("Length of circular queue is %d\n", CIRCULAR_QUEUE_SIZE);
        // get a string from the console and queue it to circular queue
        while (1)
        {
            scanf_s("%199s", inputBuffer, BUFFER_SIZE - 1); // get data from console (note this is the input from console not the circular queue yet)
            inputBuffer[BUFFER_SIZE - 1] = '\0';            // ensure the read string has a null char on end of string

            // put the data into the circular queue but check if an error (marked by queue function returning 0) occurred
            if (putToCircularQueue(inputBuffer, (unsigned int)strlen(inputBuffer)) == 0)
                printf("Error queuing data\n");
        }
    }
    else
        printf("ERROR: CreateThread Failed!\n");

    free(myQueue.ptrBuffer);                            // free up dynamically allocated memory

    return 0;
}

// FUNCTION      : putToCircularQueue
// DESCRIPTION   :
//   Put the supplied data into a circular queue
// PARAMETERS    :
//   ptrInputBuffer - a pointer to the buffer to be put into the queue
//   bufferLength   - how many characters are in the buffer being put into the queue
//
// RETURNS       :
//   Number of characters successfully queued (0 on failure)
unsigned int putToCircularQueue(char* ptrInputBuffer, unsigned int bufferLength)
{
    unsigned int numberOfCharsQueued = 0; // Counter for successfully added characters

    EnterCriticalSection(&cs); // Enter critical section to prevent concurrent modifications

    for (unsigned int i = 0; i < bufferLength; i++) { // Iterate through each character in the input buffer
        if (myQueue.queueCount == CIRCULAR_QUEUE_SIZE) { // Check if the queue is full
            // Exit loop if no space is available
            break;
        }

        *(myQueue.ptrCircularHead) = ptrInputBuffer[i]; // Add the character to the circular queue at the head pointer
        myQueue.ptrCircularHead++; // Move the head pointer to the next position

        // Wrap around the head pointer if it reaches the end of the buffer
        if (myQueue.ptrCircularHead >= myQueue.ptrBuffer + CIRCULAR_QUEUE_SIZE) {
            myQueue.ptrCircularHead = myQueue.ptrBuffer; // Reset head to the start of the buffer
        }

        myQueue.queueCount++; // Increment the count of items in the queue
        numberOfCharsQueued++; // Increment the number of characters added successfully
    }

    LeaveCriticalSection(&cs); // Exit critical section
    return numberOfCharsQueued; // Return the number of characters added to the queue
}

// FUNCTION      : getFromCircularQueue
// DESCRIPTION   :
//   Read as much data as there is room for from the circular queue
// PARAMETERS    :
//   ptrOutputBuffer - a pointer to the buffer to place the data read from queue
//   bufferLength    - maximum number of characters that can be read (size of the output buffer)
//
// RETURNS       :
//   Number of characters successfully dequeued (0 if none are read)
unsigned int getFromCircularQueue(char* ptrOutputBuffer, unsigned int bufferLength)
{
    unsigned int readCount = 0; // Counter for successfully removed characters

    EnterCriticalSection(&cs); // Enter critical section to prevent concurrent modifications

    while (readCount < bufferLength && myQueue.queueCount > 0) { // Continue until either buffer is full or queue is empty
        ptrOutputBuffer[readCount] = *(myQueue.ptrCircularTail); // Copy data from the queue tail to the output buffer
        myQueue.ptrCircularTail++; // Move the tail pointer to the next position

        // Wrap around the tail pointer if it reaches the end of the buffer
        if (myQueue.ptrCircularTail >= myQueue.ptrBuffer + CIRCULAR_QUEUE_SIZE) {
            myQueue.ptrCircularTail = myQueue.ptrBuffer; // Reset tail to the start of the buffer
        }

        myQueue.queueCount--; // Decrement the count of items in the queue
        readCount++; // Increment the number of characters removed successfully
    }

    LeaveCriticalSection(&cs); // Exit critical section
    return readCount; // Return the number of characters removed from the queue
}


// FUNCTION      : myReadThreadFunction
// DESCRIPTION   :
//   A separate thread from the main program that will independently check for data in queue and print it out
// PARAMETERS    :
//   lpParam - argument passed into thread (not used in this example)
//
// RETURNS       :
//   Will never return so no effective return value
DWORD WINAPI myReadThreadFunction(LPVOID lpParam)
{
    char readBuffer[BUFFER_SIZE];     // Local buffer to store dequeued data
    unsigned int readCount = 0;      // Counter for the number of characters dequeued

    while (1)
    {
        Sleep(INPUT_TIMEOUT_MS);      // Wait for the specified timeout period

        // Dequeue data from the circular queue
        readCount = getFromCircularQueue(readBuffer, BUFFER_SIZE - 1);

        readBuffer[readCount] = '\0'; // Null-terminate the dequeued string
        if (readCount != 0)           // Check if any data was dequeued
            printf("UNQUEUE: %s\n", readBuffer); // Print the dequeued data
    }
    return 0;    // Will never reach here
}
