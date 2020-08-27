/*
    The sorting program to use for Operating Systems Assignment 1 2020
    written by Robert Sheehan

    Modified by: Jayson Tai
    UPI: jtai406

    By submitting a program you are claiming that you and only you have made
    adjustments and additions to this code.
 */

#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <string.h>
#include <sys/resource.h>
#include <stdbool.h>
#include <sys/times.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

bool isThreadRunning = false;
bool infiniteLoop = true;

#define SIZE    10

struct block {
    int size;
    int *data; //pointer
};
struct block bufferBlock;

void print_data(struct block my_data) {
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}

/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block my_data) {
    int right = my_data.size - 1;
    int left = 0;
    int pivot = my_data.data[right]; // takes far right as pivot
    while (left < right) {
        int value = my_data.data[right - 1];
        if (value > pivot) {
            my_data.data[right--] = value; //it assigns then does the decrement
        } else {
            my_data.data[right - 1] = my_data.data[left];
            my_data.data[left++] = value; // assigns then does the increment.
        }
    }
    my_data.data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void* quick_sort(void *args) {
    struct block my_data = *(struct block *)args;
    // printf("message \n\n");
    if (my_data.size < 2)
        return NULL;

    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;
    pthread_mutex_lock(&lock);
    if (isThreadRunning == false){
        bufferBlock = left_side;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }
    else{
        // printf("The thread was busy \n\n\n");
        pthread_mutex_unlock(&lock); 
        quick_sort(&left_side);
    }

    quick_sort(&right_side);
    
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block my_data) {
    bool sorted = true;
    for (int i = 0; i < my_data.size - 1; i++) {
        if (my_data.data[i] > my_data.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block my_data) {
    srand(1); // the same random data seed every time, make random number with seed
    for (int i = 0; i < my_data.size; i++) {
        my_data.data[i] = rand() % 1000;
    }
}

void *secondThreadFunction(void *args)
{
    
    while (infiniteLoop)
    {
        if (isThreadRunning == true)
        {
            // printf("Thread not busy\n\n");
            pthread_cond_wait(&cond, &lock);
        }
        else
        {   
            // printf("Thread is busy\n\n");
            
            pthread_mutex_unlock(&lock);
            isThreadRunning = true;
            quick_sort(&bufferBlock);
            isThreadRunning = false;
            pthread_mutex_lock(&lock);
        }
    }
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

// function that gets a pivot
// sort top on one thread
// sort bottom half on main thread.
//merge both sorts together for sort.
int main(int argc, char *argv[]) {
	long size;

	if (argc < 2) {
		size = SIZE;
	} else {
		size = atol(argv[1]); //making a string into long
	}
    struct block start_block;
    start_block.size = size;
    start_block.data = (int *)calloc(size, sizeof(int)); //This statement allocates contiguous space in memory for size elements each with the size of the int.
    if (start_block.data == NULL) {
        printf("Problem allocating memory.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(start_block); //allocate random numbers to the block of memory you allocated.

    if (start_block.size < 1001)
        print_data(start_block); // just print the data.

    struct tms start_times, finish_times;
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);
    // if (my_data.size < 2)
    //     return;
    int pivot_pos = split_on_pivot(start_block);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = start_block.data;
    right_side.size = start_block.size - pivot_pos - 1;
    right_side.data = start_block.data + pivot_pos + 1;

    pthread_t newThread;
    
    if (pthread_create(&newThread, NULL, secondThreadFunction, NULL)) {
        perror("Thread being created");
        exit(EXIT_FAILURE);
    }
    quick_sort(&start_block);

    pthread_mutex_lock(&lock);
    infiniteLoop = false;
    pthread_mutex_unlock(&lock);
    pthread_cond_signal(&cond);

    pthread_join(newThread, NULL);
    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    free(start_block.data);
    exit(EXIT_SUCCESS);
}