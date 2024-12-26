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

// Merge two sorted arrays
void merge(int *arr, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int i = 0; i < n2; i++)
        R[i] = arr[mid + 1 + i];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
    {
        if (L[i] <= R[j])
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1)
        arr[k++] = L[i++];
    while (j < n2)
        arr[k++] = R[j++];

    free(L);
    free(R);
}

// Parallel Quick Sort using MPI
void parallel_quick_sort(int arr[], int size, int rank, int num_procs)
{
    int chunk_size = size / num_procs;
    int remainder = size % num_procs;
    int *sub_array;
    int *full_array = NULL;

    // Adjust for uneven sizes
    if (rank == 0)
    {
        full_array = (int *)malloc((chunk_size + remainder) * sizeof(int));
        for (int i = 0; i < size; i++)
            full_array[i] = arr[i];
    }

    // Allocate memory for sub-array
    sub_array = (int *)malloc((chunk_size + (rank == 0 ? remainder : 0)) * sizeof(int));

    // Scatter the array with remainder handled by rank 0
    if (rank == 0)
    {
        for (int i = 1; i < num_procs; i++)
        {
            MPI_Send(full_array + i * chunk_size + remainder, chunk_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
        for (int i = 0; i < chunk_size + remainder; i++)
            sub_array[i] = full_array[i];
    }
    else
    {
        MPI_Recv(sub_array, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Sort the sub-array
    quick_sort(sub_array, 0, rank == 0 ? chunk_size + remainder - 1 : chunk_size - 1);

    // Gather sorted sub-arrays back to rank 0
    if (rank == 0)
    {
        for (int i = 1; i < num_procs; i++)
        {
            MPI_Recv(arr + i * chunk_size + remainder, chunk_size, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        for (int i = 0; i < chunk_size + remainder; i++)
            arr[i] = sub_array[i];

        // Merge sorted sub-arrays
        for (int i = 1; i < num_procs; i++)
        {
            merge(arr, 0, i * chunk_size + remainder - 1, (i + 1) * chunk_size + remainder - 1);
        }

        free(full_array);
    }
    else
    {
        MPI_Send(sub_array, chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    free(sub_array);
}

void print_array(int arr[], int size)
{
    for (int i = 0; i < size; i++)
    {
        printf("%d ", arr[i]);
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    int rank, num_procs;
    int arr[] = {5, 2, 9, 1, 5, 6, 7, 3, 8, 4};
    int size = sizeof(arr) / sizeof(arr[0]);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    if (rank == 0)
    {
        printf("Original Array: ");
        print_array(arr, size);
    }

    parallel_quick_sort(arr, size, rank, num_procs);

    if (rank == 0)
    {
        printf("Sorted Array: ");
        print_array(arr, size);
    }

    MPI_Finalize();
    return 0;
}
