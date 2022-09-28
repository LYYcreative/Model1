#include <iostream>
#include <mpi.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>
#include <time.h>
#include <chrono>


using namespace std::chrono;
using namespace std;

void vectorAdd(int vector[], int size)
{
	int num = 0;
	for(int i = 0; i < size; i++)
	{
		num = i;
        // to create a set of numbers that is neither increasing nor decrementing
		if(num %2 == 0)
		{
			vector[i] = num -1;
		}
		else
		{
			vector[i] = num+1;
		}
	}
}

void qsort(int left, int right, int arr[])
{
    if(left>right){
        return;
    }
    int i,j,base,temp;
    i = left; j = right;
    base = arr[left];

    while(i<j)
    {
        while(arr[j] >= base && i<j)
            j--;
        while(arr[j] <= base && i<j)
            i++;

        if(i<j)
        {
            temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;

        }
    }
    arr[left] = arr[i];
    arr[i] = base;
    qsort(left, i-1, arr);
    qsort(i+1, right, arr);
}

void Merge(int left, int right, int arr[])
{
    int mid = (left+right)/2;
    
    //compare values in each sub-vector and replace them to right place
    int i=left, j=mid+1, k=0; 
    int *temp = (int *) malloc((right-left+1) * sizeof(int *));
    
    while(i<=mid&&j<=right)
    {
        if(arr[i]<=arr[j]) 
            temp[k++]=arr[i++];
        else
            temp[k++]=arr[j++];
    }
    while(i<=mid)
        temp[k++]=arr[i++];
    while(j<=right)
        temp[k++]=arr[j++];
    for(i=left,k=0;i<=right;i++,k++)
		arr[i]=temp[k];
    delete []temp;

    return;

}

void Print(int arr[], int size)
{
    for(int i = 0; i<size; i++)
    {
        cout << arr[i] << " ";
    }
}

int main(int argc, char** argv)
{
    int size = 16;
    int *v1, *sub_v1, *result;

    int numtasks, rank, name_len, tag = 1;
    int res;

    MPI_Status status;
    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int length = size/numtasks;

    sub_v1 = (int *) malloc(length * sizeof(int *));

    if(rank == 0)
    {
        v1 = (int *) malloc(size * sizeof(int *));
        result = (int *) malloc(size * sizeof(int *));

        vectorAdd(v1,size);
        cout << "Origenal Vector: ";
        Print(v1,size);

    }
    auto start = high_resolution_clock::now();

    MPI_Scatter(v1, length, MPI_INT, sub_v1, length, MPI_INT, 0,MPI_COMM_WORLD);

    qsort(0,length-1,sub_v1);

    MPI_Gather(sub_v1, length, MPI_INT, result, length, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0)
    {
        Merge(0,(length*2)-1,result);
        Merge(length*2,size-1,result);
        Merge(0,size-1,result);

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);
        
        cout << "\n" << "Sorted Vector: ";
        Print(result,size);
        cout << "\n";

        cout << "Time taken by function: "
         << duration.count() << " microseconds"
         << endl;
    }
    

    MPI_Finalize();

}
