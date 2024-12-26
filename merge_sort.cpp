#include <mpi.h>
#include <iostream>
#include <vector>
#include <algorithm>

// Function to merge two sorted subarrays into one sorted subarray
void merge(std::vector<int> &arr, int left, int mid, int right)
{
    int n1 = mid - left + 1;
    int n2 = right - mid;

    std::vector<int> leftArr(n1), rightArr(n2);

    for (int i = 0; i < n1; i++)
    {
        leftArr[i] = arr[left + i];
    }
    for (int i = 0; i < n2; i++)
    {
        rightArr[i] = arr[mid + 1 + i];
    }

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2)
    {
        if (leftArr[i] <= rightArr[j])
        {
            arr[k] = leftArr[i];
            i++;
        }
        else
        {
            arr[k] = rightArr[j];
            j++;
        }
        k++;
    }

    while (i < n1)
    {
        arr[k] = leftArr[i];
        i++;
        k++;
    }

    while (j < n2)
    {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
}

// Merge Sort function
void mergeSort(std::vector<int> &arr, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);      // Sort left half
        mergeSort(arr, mid + 1, right); // Sort right half
        merge(arr, left, mid, right);   // Merge both halves
    }
}

// Parallel Merge Sort using MPI
void parallelMergeSort(std::vector<int> &arr, int left, int right, int rank, int size)
{
    int n = right - left + 1;
    int elements_per_proc = n / size;
    std::vector<int> local_array(elements_per_proc);

    // Scatter the data to all processes
    MPI_Scatter(arr.data() + left, elements_per_proc, MPI_INT,
                local_array.data(), elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Sort local array in parallel
    std::sort(local_array.begin(), local_array.end());

    // Gather the sorted subarrays back to the root process
    MPI_Gather(local_array.data(), elements_per_proc, MPI_INT,
               arr.data() + left, elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    // Perform the merge operation
    if (rank == 0)
    {
        // Perform a final merge at the root process
        mergeSort(arr, left, right);
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

    // Call the parallel merge sort
    parallelMergeSort(array, 0, N - 1, rank, size);

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
