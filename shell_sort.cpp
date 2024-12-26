#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>

// Function to perform Shell Sort
void shellSort(std::vector<int> &arr)
{
    int n = arr.size();
    for (int gap = n / 2; gap > 0; gap /= 2)
    {
        for (int i = gap; i < n; i++)
        {
            int temp = arr[i];
            int j = i;
            while (j >= gap && arr[j - gap] > temp)
            {
                arr[j] = arr[j - gap];
                j -= gap;
            }
            arr[j] = temp;
        }
    }
}

// Parallel Shell Sort function using MPI
void parallelShellSort(std::vector<int> &arr, int left, int right, int rank, int size)
{
    int n = right - left + 1;
    int elements_per_proc = n / size;
    std::vector<int> local_array(elements_per_proc);

    // Scatter the data to all processes
    MPI_Scatter(arr.data() + left, elements_per_proc, MPI_INT,
                local_array.data(), elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform Shell Sort on each local portion of the array
    shellSort(local_array);

    // Gather the sorted subarrays back to the root process
    MPI_Gather(local_array.data(), elements_per_proc, MPI_INT,
               arr.data() + left, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform final sorting at the root process
    if (rank == 0)
    {
        // You can either run another pass of Shell Sort on the entire array
        // or just return as the chunks are already sorted.
        shellSort(arr);
    }
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const int N = 8; // Array size
    std::vector<int> array;

    if (rank == 0)
    {
        // Initialize the array
        array = {7, 3, 2, 9, 5, 8, 1, 6};
        std::cout << "Unsorted Array: ";
        for (int x : array)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    // Call the parallel shell sort
    parallelShellSort(array, 0, N - 1, rank, size);

    // Print the sorted array at root process
    if (rank == 0)
    {
        std::cout << "Sorted Array: ";
        for (int x : array)
            std::cout << x << " ";
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
