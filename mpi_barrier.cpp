#include <mpi.h>
#include <iostream>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    std::cout << "Node " << rank << " reached before barrier\n";
    MPI_Barrier(MPI_COMM_WORLD);
    std::cout << "Node " << rank << " reached after barrier\n ";
    MPI_Finalize();
    return 0;
}