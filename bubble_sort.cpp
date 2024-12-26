#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void bubbleSort(int *arr, int n)
{
    for (int i = 0; i < n - 1; ++i)
    {
        for (int j = 0; j < n - i - 1; ++j)
        {
            if (arr[j] > arr[j + 1])
            {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void mergeAndSort(int *local, int local_size, const int *neighbor, int neighbor_size, int isLow)
{
    int *merged = (int *)malloc((local_size + neighbor_size) * sizeof(int));
    int i = 0, j = 0, k = 0;

    // Merge the two arrays
    while (i < local_size && j < neighbor_size)
    {
        if (local[i] < neighbor[j])
        {
            merged[k++] = local[i++];
        }
        else
        {
            merged[k++] = neighbor[j++];
        }
    }
    while (i < local_size)
    {
        merged[k++] = local[i++];
    }
    while (j < neighbor_size)
    {
        merged[k++] = neighbor[j++];
    }

    // Copy the relevant portion back to local array
    if (isLow)
    {
        memcpy(local, merged, local_size * sizeof(int));
    }
    else
    {
        memcpy(local, merged + (k - local_size), local_size * sizeof(int));
    }

    free(merged);
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 8; // Array size
    int *array = NULL;

    if (rank == 0)
    {
        // Initialize the array
        array = (int *)malloc(N * sizeof(int));
        int temp[] = {7, 3, 2, 9, 5, 8, 1, 6};
        memcpy(array, temp, N * sizeof(int));
        printf("Unsorted Array: ");
        for (int i = 0; i < N; i++)
        {
            printf("%d ", array[i]);
        }
        printf("\n");
    }

    int elements_per_proc = N / size;
    int *local_array = (int *)malloc(elements_per_proc * sizeof(int));

    // Scatter data to all processes
    MPI_Scatter(array, elements_per_proc, MPI_INT, local_array, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Sort local data
    bubbleSort(local_array, elements_per_proc);

    // Perform parallel odd-even transposition sort
    for (int phase = 0; phase < size; ++phase)
    {
        int partner = (phase % 2 == 0) ? ((rank % 2 == 0) ? rank + 1 : rank - 1)
                                       : ((rank % 2 == 0) ? rank - 1 : rank + 1);

        if (partner >= 0 && partner < size)
        {
            int *neighbor_array = (int *)malloc(elements_per_proc * sizeof(int));
            MPI_Sendrecv(local_array, elements_per_proc, MPI_INT, partner, 0,
                         neighbor_array, elements_per_proc, MPI_INT, partner, 0,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            if (rank < partner)
            {
                mergeAndSort(local_array, elements_per_proc, neighbor_array, elements_per_proc, 1);
            }
            else
            {
                mergeAndSort(local_array, elements_per_proc, neighbor_array, elements_per_proc, 0);
            }

            free(neighbor_array);
        }
    }

    // Gather sorted chunks at root
    MPI_Gather(local_array, elements_per_proc, MPI_INT, array, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        printf("Sorted Array: ");
        for (int i = 0; i < N; i++)
        {
            printf("%d ", array[i]);
        }
        printf("\n");
        free(array);
    }

    free(local_array);
    MPI_Finalize();
    return 0;
}