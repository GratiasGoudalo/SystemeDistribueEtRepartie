#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>

/* Magic Number for Key */
#define MAGIC_NUMBER    1111

/* Global Array for A, B */
unsigned int A[800][800], B[800][800];
unsigned int *C;

/* Function Definitions */
unsigned long long processOne(int n);
unsigned long long processTwo(int n);
unsigned long long processFour(int n);
void initMemory();

/* Global Variables for shared memory ID */
int shmidC;

/* Self-write Functions for Attaching Shared Memories */
unsigned int *attachSharedMemoryC(int n);
void detachSharedMemory();
void detachAndReleaseSharedMemory();

/* Main */
int main() {

    // inits
    unsigned long long sum = 0;
    int n, sec, usec;
    struct timeval start, end;
    printf("Entrer la dimension de la matrice: ");
    scanf("%d", &n);

    initMemory(n);

    // 1-partition
    gettimeofday(&start, 0);
    sum = processOne(n);
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Multiplying matrices by 1 process: %f sec\n", sec + (usec/1000000.0));
    printf("Element Sum: %llu\n\n", sum);

    // 2-partition
    gettimeofday(&start, 0);
    sum = processTwo(n);
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Multiplying matrices by 2 processes: %f sec\n", sec + (usec/1000000.0));
    printf("Element Sum: %llu\n\n", sum);

    // 4-partition
    gettimeofday(&start, 0);
    sum = processFour(n);
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    printf("Multiplying matrices by 3 processes: %f sec\n", sec + (usec/1000000.0));
    printf("Element Sum: %llu\n\n", sum);

    //
    return 0;
}

void initMemory(int n){
    int i, j;
    for( i=0 ; i<n ; i++ ){
        for( j=0 ; j<n ; j++ ){
            A[i][j] = B[i][j] = i*n + j;
        }
    }
}

unsigned long long processOne(int n){
    int i, j, k;
    unsigned long long sum = 0;
    // calculate C
    for( i=0 ; i<n ; i++ ){
        for( j=0 ; j<n ; j++ ){
            unsigned int tc = 0;
            for( k=0 ; k<n ; k++ ){
                tc += (A[i][k] * B[k][j]);
            }
            sum += tc;
        }
    }
    return sum;
}

unsigned long long processTwo(int n){

    int i, j, k;
    C  = attachSharedMemoryC(n);

    // sum saves here
    unsigned long long sum = 0;

    // fork here
    pid_t pid;
    pid = fork();

    if(pid <0) {
        // if error, fork failed
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if(pid == 0){
        // child, and calc the first part, i=0 ~ n/2
        for( i=0 ; i<n/2 ; i++ ){
            for( j=0 ; j<n ; j++ ){
                C[i*n + j] = 0;
                for( k=0 ; k<n ; k++ ){
                    C[i*n + j] += A[i][k] * B[k][j];
                }
            }
        }

        detachSharedMemory();
        exit(0);
    }
    else {
        // parent, i=n/2 ~ n
        for( i=n/2 ; i<n ; i++ ){
            for( j=0 ; j<n ; j++ ){
                C[i*n + j] =0;
                for( k=0 ; k<n ; k++ ){
                    C[i*n + j] += A[i][k] * B[k][j];
                }
            }
        }

        // if parent
        wait(NULL);

        // sum up here
        for( i=0 ; i<n*n ; i++ ) sum += C[i];
        detachAndReleaseSharedMemory();
    }

    return sum;
}

unsigned long long processFour(int n){

    int i, j, k;
    unsigned long long sum = 0;
    C  = attachSharedMemoryC(n);

    // fork here
    pid_t pid;
    pid = fork();

    if(pid <0) {
        // if error, fork failed
        fprintf(stderr, "Fork Failed");
        exit(-1);
    }
    else if(pid == 0){
        // fork here
        pid_t pid2;
        pid2 = fork();

        if(pid2 <0) {
            // if error, fork failed
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if(pid2 == 0){
            // child, and calc the first part, i=0 ~ n/2
            for( i=0 ; i<n/2 ; i++ ){
                for( j=0 ; j<n/2 ; j++ ){
                    C[i*n + j] = 0;
                    for( k=0 ; k<n ; k++ ){
                        C[i*n + j] += A[i][k] * B[k][j];
                    }
                }
            }

            detachSharedMemory();
            exit(0);
        }
        else {
            // parent, i=n/2 ~ n
            for( i=n/2 ; i<n ; i++ ){
                for( j=0 ; j<n/2 ; j++ ){
                    C[i*n + j] = 0;
                    for( k=0 ; k<n ; k++ ){
                        C[i*n + j] += A[i][k] * B[k][j];
                    }
                }
            }

            wait(NULL);
        }
        detachSharedMemory();
        exit(0);
    }
    else {
        // fork here
        pid_t pid3;
        pid3 = fork();

        if(pid3 <0) {
            // if error, fork failed
            fprintf(stderr, "Fork Failed");
            exit(-1);
        }
        else if(pid3 == 0){
            // child, and calc the first part, i=0 ~ n/2
            for( i=0 ; i<n/2 ; i++ ){
                for( j=n/2 ; j<n ; j++ ){
                    C[i*n + j] = 0;
                    for( k=0 ; k<n ; k++ ){
                        C[i*n + j] += A[i][k] * B[k][j];
                    }
                }
            }
            detachSharedMemory();
            exit(0);
        }
        else {
            // parent, i=n/2 ~ n
            for( i=n/2 ; i<n ; i++ ){
                for( j=n/2 ; j<n ; j++ ){
                    C[i*n + j] = 0;
                    for( k=0 ; k<n ; k++ ){
                        C[i*n + j] += A[i][k] * B[k][j];
                    }
                }
            }

            // if parent
            wait(NULL);
            wait(NULL);

            // sum up here
            for( i=0 ; i<n*n ; i++ ) sum += C[i];
            detachAndReleaseSharedMemory();
        }
    }

    return sum;
}

void detachAndReleaseSharedMemory(){
    detachSharedMemory();
    if (shmctl(shmidC, IPC_RMID, 0) == -1) {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(1);
    }
}

void detachSharedMemory(){
    if (shmdt(C) == -1) {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
}

unsigned int *attachSharedMemoryC(int n){
    // setup shared memory
    key_t key = MAGIC_NUMBER;
    int shmflg = IPC_CREAT | 0666;

    if((shmidC = shmget(key, sizeof(unsigned int) * n * n, shmflg)) == -1) {
        perror("shmget: shmget failed");
        exit(1);
    }

    unsigned int *sC;
    if((sC = shmat(shmidC, NULL, 0)) == (unsigned int*)-1) {
        perror("shmat: attach error");
        exit(1);
    }

    return sC;
}
