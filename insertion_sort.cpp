#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function to perform insertion sort on an array
void insertion_sort(int arr[], int size)
{
    for (int i = 1; i < size; i++)
    {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

// Merge two sorted sub-arrays into a single sorted array
void merge(int *result, int *left, int left_size, int *right, int right_size)
{
    int i = 0, j = 0, k = 0;
    while (i < left_size && j < right_size)
    {
        if (left[i] <= right[j])
        {
            result[k++] = left[i++];
        }
        else
        {
            result[k++] = right[j++];
        }
    }
    while (i < left_size)
        result[k++] = left[i++];
    while (j < right_size)
        result[k++] = right[j++];
}

// Parallel Insertion Sort using MPI
void parallel_insertion_sort(int arr[], int size, int rank, int num_procs)
{
    int chunk_size = size / num_procs;
    int remainder = size % num_procs; // For handling uneven chunk sizes
    int *sub_array;
    int local_size;

    // Calculate chunk size for each process
    if (rank < remainder)
    {
        local_size = chunk_size + 1;
    }
    else
    {
        local_size = chunk_size;
    }

    // Allocate memory for sub-array
    sub_array = (int *)malloc(local_size * sizeof(int));
    if (sub_array == NULL)
    {
        printf("Error: Memory allocation failed on process %d\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Scatter the array to all processes
    int *send_counts = NULL;
    int *displacements = NULL;

    if (rank == 0)
    {
        send_counts = (int *)malloc(num_procs * sizeof(int));
        displacements = (int *)malloc(num_procs * sizeof(int));
        for (int i = 0; i < num_procs; i++)
        {
            send_counts[i] = (i < remainder) ? chunk_size + 1 : chunk_size;
            displacements[i] = (i == 0) ? 0 : displacements[i - 1] + send_counts[i - 1];
        }
    }

    MPI_Scatterv(arr, send_counts, displacements, MPI_INT, sub_array, local_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Each process performs insertion sort on its sub-array
    insertion_sort(sub_array, local_size);

    // Gather the sorted sub-arrays back at root process
    int *sorted_array = NULL;
    if (rank == 0)
    {
        sorted_array = (int *)malloc(size * sizeof(int));
    }

    MPI_Gatherv(sub_array, local_size, MPI_INT, sorted_array, send_counts, displacements, MPI_INT, 0, MPI_COMM_WORLD);

    // Merge sorted sub-arrays at root process
    if (rank == 0)
    {
        int *temp = (int *)malloc(size * sizeof(int));
        int *current = sorted_array;
        int current_size = send_counts[0];

        for (int i = 1; i < num_procs; i++)
        {
            merge(temp, current, current_size, sorted_array + displacements[i], send_counts[i]);
            for (int j = 0; j < current_size + send_counts[i]; j++)
            {
                sorted_array[j] = temp[j];
            }
            current_size += send_counts[i];
        }

        printf("Sorted Array: ");
        for (int i = 0; i < size; i++)
        {
            printf("%d ", sorted_array[i]);
        }
        printf("\n");

        free(temp);
        free(send_counts);
        free(displacements);
        free(sorted_array);
    }

    free(sub_array);
}

void print_array(const char *message, int arr[], int size)
{
    printf("%s: ", message);
    for (int i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int rank, num_procs;
    int arr[] = {15, 2, 9, 25, 33, 9, 1, 5, 6, 7, 3, 8, 42, 45, 4};
    int size = sizeof(arr) / sizeof(arr[0]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
    {
        print_array("Unsorted Array", arr, size); // Print the original array
    }

    parallel_insertion_sort(arr, size, rank, num_procs);

    MPI_Finalize();
    return 0;
}
