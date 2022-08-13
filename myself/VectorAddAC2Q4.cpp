#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>
#include <stdlib.h>


using namespace std::chrono;
using namespace std;



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

#pragma omp paralell num threads(8) private(size,v1,v2,v3) shared(total2) default(none)
    {
        randomVector(v1, size);
        randomVector(v2, size);

        #pragma omp for
        for (int i = 0; i < size; i++)
        {
            v3[i] = v1[i] + v2[i];
        }

        #pragma omp critical
        {
            for(int i = 0; i< size; i++){
                total2 += v3[i];
            }

        }
   
        // g++ -fopenmp \VectorAdd.cpp -o filea.exe

    }

    cout << "Paralell sum of vector 3: "
    << total2 << endl;

    
    auto stop = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;

    return 0;
}