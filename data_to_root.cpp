#include <mpi.h>
#include <iostream>
#include <vector>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    const int N = 10;
    std::vector<int> data(N, rank); // Each node generates an array with its rank as data
    if (rank != 0)
    {
        MPI_Send(data.data(), N, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    else
    {
        std::vector<int> received_data(N);
        for (int i = 1; i < size; ++i)
        {
            MPI_Recv(received_data.data(), N, MPI_INT, i, 0, MPI_COMM_WORLD,
                     MPI_STATUS_IGNORE);
            std::cout << "Received data from node " << i << ":";
            for (int x : received_data)
                std::cout << x << "";
            std::cout << "\n ";
        }
    }
    MPI_Finalize();
    return 0;
}