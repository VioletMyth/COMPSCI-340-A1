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
#include <sys/mman.h>
#include <sys/wait.h>

#define SIZE 10

int minSize = 200000;

struct block
{
    int size;
    int *data; //pointer
};

void print_data(struct block my_data)
{
    for (int i = 0; i < my_data.size; ++i)
        printf("%d ", my_data.data[i]);
    printf("\n");
}

/* Split the shared array around the pivot, return pivot index. */
int split_on_pivot(struct block my_data)
{
    int right = my_data.size - 1;
    int left = 0;
    int pivot = my_data.data[right]; // takes far right as pivot
    while (left < right)
    {
        int value = my_data.data[right - 1];
        if (value > pivot)
        {
            my_data.data[right--] = value; //it assigns then does the decrement
        }
        else
        {
            my_data.data[right - 1] = my_data.data[left];
            my_data.data[left++] = value; // assigns then does the increment.
        }
    }
    my_data.data[right] = pivot;
    return right;
}

/* Quick sort the data. */
void *quick_sort(void *args)
{
    struct block my_data = *(struct block *)args;

    if (my_data.size < 2)
        return NULL;
    int pivot_pos = split_on_pivot(my_data);

    struct block left_side, right_side;

    left_side.size = pivot_pos;
    left_side.data = my_data.data;
    right_side.size = my_data.size - pivot_pos - 1;
    right_side.data = my_data.data + pivot_pos + 1;

    if (left_side.size > minSize || right_side.size > minSize)
    {

        int pid2 = fork();

        if (pid2 == -1)
        {
            printf("Error occurred in creating fork");
        }

        if (pid2 == 0)
        {
            quick_sort(&left_side);
            exit(EXIT_SUCCESS);
        }

        else
        {
            quick_sort(&right_side);
            wait(NULL); // wait for child process to finish
        }
    }
    else
    {
        quick_sort(&left_side);
        quick_sort(&right_side);
    }
}

/* Check to see if the data is sorted. */
bool is_sorted(struct block my_data)
{
    bool sorted = true;
    for (int i = 0; i < my_data.size - 1; i++)
    {
        if (my_data.data[i] > my_data.data[i + 1])
            sorted = false;
    }
    return sorted;
}

/* Fill the array with random data. */
void produce_random_data(struct block my_data)
{
    srand(1); // the same random data seed every time, make random number with seed
    for (int i = 0; i < my_data.size; i++)
    {
        my_data.data[i] = rand() % 1000;
    }
}
// function that gets a pivot
// sort top on one thread
// sort bottom half on main thread.
//merge both sorts together for sort.
int main(int argc, char *argv[])
{
    long size;

    if (argc < 2)
    {
        size = SIZE;
    }
    else
    {
        size = atol(argv[1]); //making a string into long
    }
    struct block start_block;
    start_block.size = size;
    start_block.data = (int *)mmap(NULL, start_block.size * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0); //This statement allocates contiguous space in memory for size elements each with the size of the int.
    if (start_block.data == MAP_FAILED)
    {
        printf("Problem allocating memory.\n");
        exit(EXIT_FAILURE);
    }

    produce_random_data(start_block); //allocate random numbers to the block of memory you allocated.

    if (start_block.size < 1001)
        print_data(start_block); // just print the data.

    struct tms start_times, finish_times;
    times(&start_times);
    printf("start time in clock ticks: %ld\n", start_times.tms_utime);

    quick_sort(&start_block);
    times(&finish_times);
    printf("finish time in clock ticks: %ld\n", finish_times.tms_utime);
    if (start_block.size < 1001)
        print_data(start_block);

    printf(is_sorted(start_block) ? "sorted\n" : "not sorted\n");
    // free(start_block.data);
    exit(EXIT_SUCCESS);
    // rest of the main() process.
}