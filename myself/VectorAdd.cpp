#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>
#include <stdlib.h>


using namespace std::chrono;
using namespace std;

int calculatesum (int* vector, int size){
    int totalSum = 0;
#pragma omp paralell for reduction(+: totalSum)
    for(int i = 0; i< size; i++)
    {
        #pragma omp atomic update
        totalSum += vector[i];

    }

    return totalSum;

}


void randomVector(int vector[], int size)
{
    #pragma omp for
    for (int i = 0; i < size; i++)
    {
        uint seed = 1;
        vector[i] = rand_r(&seed) % 100;
    }
 
}


int main(){

    unsigned long size = 100000000;

    srand(time(0));

    int *v1, *v2, *v3;

    int total2;


    auto start = high_resolution_clock::now();
    

    v1 = (int *) malloc(size * sizeof(int *));
    v2 = (int *) malloc(size * sizeof(int *));
    v3 = (int *) malloc(size * sizeof(int *));

#pragma omp paralell num threads(8) shared(v1,v3,v2) private(size) default(none)
    {
        randomVector(v1, size);
        randomVector(v2, size);

        #pragma omp for
        for (int i = 0; i < size; i++)
        {
            v3[i] = v1[i] + v2[i];
        }
   
        // g++ -fopenmp \VectorAdd.cpp -o filea.exe

    }

    total2 = calculatesum(v3,size);
    cout << "Paralell sum of vector 3: "
    << total2 << endl;

    
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}
