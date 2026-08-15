#include <stdio.h>
#include <mpi.h>

int main() {
    int rank;
    int numprocs;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    /* set up timer */
    double starttime;
    double endtime;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    starttime = MPI_Wtime();

    /* retrieve processor name */
    MPI_Get_processor_name(processor_name, &name_len);
    
    printf("[%s] Hello, World! from rank %d\n", processor_name, rank);

    endtime = MPI_Wtime();

    /* print timer */
    printf("[%s] rank %d spends time %f seconds\n", processor_name, rank, endtime - starttime);

    /* print number of processes on rank 0 only */
    if (rank == 0) {
        printf("number of procs: %d\n", numprocs);
    }

    MPI_Finalize();

    return 0;
}
