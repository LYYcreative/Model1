#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <pthread.h>

#define NUM_THREADS 4

using namespace std::chrono;
using namespace std;

struct sortTask
{
    int *v1;
    int left;
    int right;
    int partitionSize;
};

//creates an initial unordered vector
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

//quick sort
void *quickSort(void *args)
{
    //initial the struct sortTask
    sortTask *task = ((struct sortTask *)args);
    
    int Left = task->left;
    int Right = task->right;
    int check = Right-Left;

    if(Left >= Right)
		return NULL;

	int i, j, base, temp;
	i = Left, j = Right;
	base = task->v1[Left];//let the left most element as basis

    //j moving from right to left, i moving from left to right
	//if find an array[j] less than base-value and an array[i] greater than base-value, swap array[i] and array[j]
	while (i < j)
	{
		while (task->v1[j] >= base && i < j)
			j--;
		while (task->v1[i] <= base && i < j)
			i++;
		if(i < j)
		{
			temp = task->v1[i];
			task->v1[i] = task->v1[j];
			task->v1[j] = temp;
		}
	}
    //puts the base value to the right place
	task->v1[Left] = task->v1[i];
	task->v1[i] = base;

    //prepare correct values for recursion and do recursion
    task->right = i-1;
    task->left = Left;
	quickSort(task);
    task->right = Right;
    task->left = i+1;
	quickSort(task);
}

//merge any two sequential parts
void *Merge(void *args)
{
    sortTask *task = ((struct sortTask *)args);
    int Right = task->right;//the index of the first element in the first sequential part
    int Left = task->left;//the index of the last element in the second sequential part
    int mid = (Left+Right)/2;
    
    //compare values in each sub-vector and replace them to right place
    int i=Left, j=mid+1, k=0; 
    int *temp=new(nothrow) int[Right-Left+1]; 
    
    while(i<=mid&&j<=Right)
    {
        if(task->v1[i]<=task->v1[j]) 
            temp[k++]=task->v1[i++];
        else
            temp[k++]=task->v1[j++];
    }
    while(i<=mid)
        temp[k++]=task->v1[i++];
    while(j<=Right)
        temp[k++]=task->v1[j++];
    for(i=Left,k=0;i<=Right;i++,k++)
		task->v1[i]=temp[k];
    delete []temp;

    return NULL;

}

int main()
{

    int size = 30000;

    srand(time(0));
    //initial vector v1
    int *v1;
	v1 = (int *) malloc(size * sizeof(int *));
	vectorAdd(v1,size);

    auto start = high_resolution_clock::now();
    pthread_t sortThreads[NUM_THREADS];

    int partition_size = size / NUM_THREADS;

    //creates four threads for quickSort
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        struct sortTask *task = (struct sortTask *)malloc(sizeof(struct sortTask));
        task->v1 = v1;
        task->left = i * partition_size;
        task->right = task->left + partition_size-1;
        task->partitionSize = partition_size;
        //devides v1 to four parts and paralell sort each part
        pthread_create(&sortThreads[i], NULL, quickSort, (void *)task);
    }

    //keep waiting until all quicksort threads finish
    for (size_t i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(sortThreads[i], NULL);
    }

    //creats 2 threads for merge
    pthread_t mergeThreads[NUM_THREADS/2];
    for (size_t i = 0; i < NUM_THREADS/2; i++)
    {
        struct sortTask *task = (struct sortTask *)malloc(sizeof(struct sortTask));
        task->v1 = v1;
        task->left = i * partition_size*NUM_THREADS/2;
        task->right = task->left + (partition_size*NUM_THREADS/2)-1;
        //paralell merge 2 parts, each part has two 1/4 of v1
        pthread_create(&mergeThreads[i], NULL, Merge, (void *)task);
    }
    //keep waiting until all merge threads finish
    for (size_t i = 0; i < NUM_THREADS/2; i++)
    {
        pthread_join(mergeThreads[i], NULL);
    }
    //finally merge 2 sub-vectors, each sub-vector has half of v1
    struct sortTask *task = (struct sortTask *)malloc(sizeof(struct sortTask));
    task->v1 = v1;
    task->left = 0;
    task->right = size-1;
    Merge(task);

    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}
