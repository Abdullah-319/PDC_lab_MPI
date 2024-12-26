#include <mpi.h>
#include <iostream>
#include <vector>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get the total number of processes

    const int N = 100; // Size of the array
    std::vector<int> data;
    int local_sum = 0;

    if (rank == 0)
    {
        // Master process initializes the array
        data.resize(N);
        for (int i = 0; i < N; i++)
        {
            data[i] = i + 1; // Fill array with values 1 to N
        }

        // Distribute chunks of the array to other processes
        int chunk_size = N / size;
        for (int i = 1; i < size; i++)
        {
            MPI_Send(&data[i * chunk_size], chunk_size, MPI_INT, i, 0, MPI_COMM_WORLD);
        }

        // Master process computes its own chunk
        for (int i = 0; i < chunk_size; i++)
        {
            local_sum += data[i];
        }
    }
    else
    {
        // Worker processes receive their chunk
        int chunk_size = N / size;
        std::vector<int> local_data(chunk_size);
        MPI_Recv(local_data.data(), chunk_size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Compute the local sum
        for (int val : local_data)
        {
            local_sum += val;
        }
    }

    // Collect all local sums and compute the global sum
    int global_sum = 0;
    MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    // Master process prints the result
    if (rank == 0)
    {
        std::cout << "Total sum: " << global_sum << std::endl;
    }

    MPI_Finalize();
    return 0;
}
