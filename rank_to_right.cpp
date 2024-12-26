#include <mpi.h>
#include <iostream>
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    int send_data = rank;
    int
        recv_data = -1;
    int left = (rank - 1 + size) % size;
    int right = (rank + 1) % size;
    MPI_Request requests[2];

    MPI_Isend(&send_data, 1, MPI_INT, right, 0, MPI_COMM_WORLD,
              &requests[0]);
    MPI_Irecv(&recv_data, 1, MPI_INT, left, 0, MPI_COMM_WORLD,
              &requests[1]);
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
    std::cout << "Node " << rank << " received " << recv_data << " from node " << left << "\n";
    MPI_Finalize();
    return 0;
}