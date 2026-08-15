#include <stdio.h>
#include <mpi.h>

int main() {
    int rank;
    int numprocs;

    /* set up timer */
    double starttime;
    double endtime;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    starttime = MPI_Wtime();
    
    printf("Hello, World! from rank %d\n", rank);

    endtime = MPI_Wtime();

    /* print timer */
    printf("rank %d spends time %f seconds\n", rank, endtime - starttime);

    /* print number of processes on rank 0 only */
    if (rank == 0) {
        printf("number of procs: %d\n", numprocs);
    }

    MPI_Finalize();

    return 0;
}
