#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Function to partition the array
int partition(int arr[], int low, int high)
{
    int pivot = arr[high];
    int i = low - 1;
    for (int j = low; j < high; j++)
    {
        if (arr[j] < pivot)
        {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return (i + 1);
}

// Quick Sort function
void quick_sort(int arr[], int low, int high)
{
    if (low < high)
    {
        int pivotIndex = partition(arr, low, high);
        quick_sort(arr, low, pivotIndex - 1);
        quick_sort(arr, pivotIndex + 1, high);
    }
}

// Print array
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
    int arr[] = {15, 2, 21, 9, 17, 1, 5, 6, 7, 3, 8, 4};
    int size = sizeof(arr) / sizeof(arr[0]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int chunk_size = size / num_procs;
    int *sub_array = (int *)malloc(chunk_size * sizeof(int));

    if (rank == 0)
    {
        // Print the unsorted array
        print_array("Unsorted Array", arr, size);
    }

    // Scatter the data among processes
    MPI_Scatter(arr, chunk_size, MPI_INT, sub_array, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Sort the local chunk
    quick_sort(sub_array, 0, chunk_size - 1);

    // Gather the sorted chunks back to the root process
    int *gathered_array = NULL;
    if (rank == 0)
    {
        gathered_array = (int *)malloc(size * sizeof(int));
    }
    MPI_Gather(sub_array, chunk_size, MPI_INT, gathered_array, chunk_size, MPI_INT, 0, MPI_COMM_WORLD);

    // Ensure the global array is sorted
    if (rank == 0)
    {
        quick_sort(gathered_array, 0, size - 1);           // Final sorting to merge sorted chunks
        print_array("Sorted Array", gathered_array, size); // Print the sorted array
        free(gathered_array);
    }

    free(sub_array);
    MPI_Finalize();
    return 0;
}
