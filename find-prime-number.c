#include <errno.h>
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_DIR "/mnt/exports/output"

static void get_prime_number_count(unsigned long long int min_num, unsigned long long int max_num, unsigned long int *count, FILE *output) {
    unsigned long long int init_num = min_num;

    while (init_num <= max_num) {
        int counter = 0;

        unsigned long long int boundary = (unsigned long long int)sqrtl(init_num);

        for (unsigned long long int c = 2; c <= boundary; ++c) {
            if (init_num % c == 0) {
                ++counter;
            }

            if (counter > 0) {
                break;
            }
        }

        /* a prime number is found */
        if (counter == 0) {
            ++(*count);

            /* save prime number */
            if (output != NULL) {
                fprintf(output, "%llu\n", init_num);
            }
        }

        ++init_num;
    }
}

int main(int argc, char *argv[]) {
    /* MPI environment variables */
    int rank;
    int numprocs;
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;

    /* initialize MPI environment */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* check input params */
    if (argc < 2) {
        /* usage error should come to rank 0 only */
        if (rank == 0) {
            fprintf(stderr, "usage: %s <maximum number limit>\n", argv[0]);
        }

        /* MUST call MPI_Finalize() to destroy ALL ranks */
        MPI_Finalize();

        return 1;
    }

    /* set up timer */
    double starttime;
    double endtime;

    /* set up prime number variables */
    unsigned long long int min = 2;
    unsigned long long int max = strtoull(argv[1], NULL, 10);
    unsigned long int local_prime_number_count = 0;
    unsigned long int total_prime_number_count = 0;

    /* retrieve processor name */
    MPI_Get_processor_name(processor_name, &name_len);

    /* set up output file variables */
    FILE *prime_number_output;
    prime_number_output = NULL;

    char prime_number_output_filename[BUFSIZ];

    /* make sure output filename ALWAYS include processor name + rank number to avoid filename collision */
    snprintf(prime_number_output_filename, BUFSIZ, "%s/find-prime-number.%s.rank%d.out", OUTPUT_DIR, processor_name, rank);

    prime_number_output = fopen(prime_number_output_filename, "w");
    if (prime_number_output == NULL) {
        fprintf(stderr, "[%s] ERROR: failed to open file %s on rank %d: %s\n", processor_name, prime_number_output_filename, rank, strerror(errno));

        MPI_Finalize();
        return 1;
    }

    starttime = MPI_Wtime();

    /* calculate range of numbers based on rank */
    unsigned long long int total_numbers = (max - min + 1);
    unsigned long long int chunk_size = total_numbers / numprocs;
    unsigned long long int remainder = total_numbers % numprocs;
    unsigned long long int local_min, local_max;

    if (rank < remainder) {
        local_min = min + rank * (chunk_size + 1);
        local_max = local_min + chunk_size;
    } else {
        local_min = min + rank * chunk_size + remainder;
        local_max = local_min + chunk_size - 1;
    }

    get_prime_number_count(local_min, local_max, &local_prime_number_count, prime_number_output);

    /*
     * combine results
     *
     * 1st param: send buffer. each rank's result
     * 2nd param: receive buffer. final result
     * 3rd param: number of items in send buffer. only 1 item, which is the local prime number count
     * 4th param: data type for transfer. number of prime numbers is unsigned long int data type
     * 5th param: reduce operation. using MPI_SUM to combine all results
     * 6th param: rank of root process
     * 7th param: communicator
     */
    MPI_Reduce(&local_prime_number_count,
        &total_prime_number_count,
        1,
        MPI_UNSIGNED_LONG,
        MPI_SUM,
        0,
        MPI_COMM_WORLD);

    endtime = MPI_Wtime();

    /* print timer */
    printf("[%s] rank %d spends time %f seconds to find %lu prime number(s)\n", processor_name, rank, endtime - starttime, local_prime_number_count);

    /* flush and close stream */
    fflush(prime_number_output);
    fclose(prime_number_output);

    /* print result on rank 0 only */
    if (rank == 0) {
        printf("total number of prime numbers: %lu\n", total_prime_number_count);
    }

    MPI_Finalize();

    return 0;
}
