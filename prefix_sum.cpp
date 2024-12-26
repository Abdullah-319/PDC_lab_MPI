#include <mpi.h>
#include <iostream>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int a = rank;
    int b =
        0;
    MPI_Scan(&a, &b, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    std::cout << "Node " << rank << ": Prefix sum = " << b << "\n";
    MPI_Finalize();
    return 0;
}