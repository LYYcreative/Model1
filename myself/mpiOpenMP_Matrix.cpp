#include <stdio.h>
#include "mpi.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <chrono>
#include <iostream>
#include <cstdlib>
#include <omp.h>

using namespace std::chrono;
using namespace std;

//initial matrix
int **generate_matrix(int size)
{
    int num = 0,m;
    int **matrix;
    matrix = (int **)malloc(sizeof(int *) * size);
    for(m = 0; m < size; m++)
        matrix[m] = (int *)malloc(sizeof(int) * size);
    int i,j;

    int value = 0;
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            matrix[i][j] = value;
            value += 1;
        }
    }
    return matrix;
}
//print marix
void print_matrx(int **a,int size)
{
    int i,j;
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < size; j++)
        {
            printf("%d ",a[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}
//matrix multiplication
int * Multiplication(int **a,int b[],int size)
{
    int *result;
    result = (int *)malloc(sizeof(int) * size);
    int i,m,n,sum = 0;
    
    //using openMP parallel compute the matrix multiplication
    #pragma omp paralell num_threads(size/2)
    {
        #pragma omp for
        for(m = 0; m < size; m++)
        {
            for(n = 0; n < size; n++)
            {
                sum += a[n][m] * b[n];
            }
            result[m] = sum;
            sum = 0;
        }
    }
    
    return result;
}
int main(int argc,char **argv)
{
    //build mpi environment
    int size,rank,dest;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(comm,&size);
    MPI_Comm_rank(comm,&rank);


    int **matrix1;
    int **matrix2;
    int send_buff[size*size];
    matrix1 = generate_matrix(size);
    matrix2 = generate_matrix(size);

    
    if(rank == 0)
    {
        
        printf("matrix1 is :\n");
        print_matrx((int **)matrix1,size);

        printf("matrix2 is :\n");
        print_matrx((int **)matrix2,size);
        //create a one dimentional copy of matrix1 called send_buff
        int j,k,tmp = 0;
        for(j = 0; j < size; j++)
            for(k = 0; k < size; k++)
            {
                send_buff[tmp] = matrix1[j][k];
                tmp++;
            }
    }

    //each thread has rbuf to store results
    int rbuf[size];
    int final_buff[size];

    int *result;

    result = (int *)malloc(sizeof(int) * size);

    auto start = high_resolution_clock::now();
    
    //scatter the send_buff--one dimensional matrix1 to each thread
    //result stored in rubf
    MPI_Scatter(send_buff,size,MPI_INT,rbuf,size,MPI_INT,0,comm);

    result = Multiplication((int **)matrix2,rbuf,size);
    MPI_Barrier(comm);//waitting for all threads end

    //create a recieve vector
    int *recv_buff;
    if(rank == 0)
        recv_buff = (int*)malloc(sizeof(int)*size*size); 
    MPI_Barrier(comm);

    MPI_Gather(result,size,MPI_INT,recv_buff,size,MPI_INT,0,comm);//gather the coloums from each thread, and generate the final result

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);
    
    //print the result
    if(rank == 0)
    {
        
        printf("\nresult is :\n");
        int m,n,tmp = 0;
        for(m = 0; m < size; m++)
        {
            for(n = 0;n < size;n++)
            {
                printf("%d ",recv_buff[tmp]);
                tmp++;
            }
            printf("\n");

        }
        printf("\n");

        cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;
    }


    MPI_Finalize();
    return 0;
}
