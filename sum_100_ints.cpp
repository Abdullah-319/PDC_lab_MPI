#include <mpi.h>
#include <iostream>
#include <vector>
#include <numeric>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    const int N = 100; // Total numbers to sum
    std::vector<int> numbers(N);
    if (rank == 0)
    {
        for (int i = 0; i < N; ++i)
            numbers[i] = i + 1;
    }
    int local_sum = 0;
    int
        total_sum = 0;
    MPI_Barrier(MPI_COMM_WORLD);
    double
        start_time = MPI_Wtime();
    int chunk_size = N / size;
    std::vector<int> local_numbers(chunk_size);
    MPI_Scatter(numbers.data(), chunk_size,
                MPI_INT, local_numbers.data(), chunk_size, MPI_INT, 0, MPI_COMM_WORLD);
    local_sum = std::accumulate(local_numbers.begin(), local_numbers.end(), 0);
    MPI_Reduce(&local_sum, &total_sum, 1, MPI_INT, MPI_SUM, 0,
               MPI_COMM_WORLD);
    double end_time = MPI_Wtime();
    if (rank == 0)
    {

        std::cout << "Total Sum: " << total_sum << "\n";
        std::cout << "Execution Time: " << end_time - start_time
                  << " seconds\n";
    }
    MPI_Finalize();
    return 0;
}