#include <iostream>
#include <cstdlib>
#include <time.h>
#include <chrono>
#include <omp.h>
#include <pthread.h>
#include <stdlib.h>
#include <fstream>

using namespace std::chrono;
using namespace std;
ofstream fout;

//function to fill the matrix (the elements in matrix are from 0 to N²-1)
void fillMatrix(int r, int c, int **Mtrx)
{
    for(int i = 0; i<r; i++)
    {
        Mtrx[i] = new int[c];
    }

    int value = 0;
    for(int i = 0; i<r; i++)
    {
        for(int j = 0; j< c; j++)
        {
            Mtrx[i][j] = value;
            value += 1;
        }
    }
}

// function to write a matrix to output.txt
void Print(int r, int c, int **Mtrx, string a)
{
    fout << a;
    for(int i = 0; i<r; i++)
    {
        for(int j = 0; j< c; j++)
        {
            fout << Mtrx[i][j]<< " ";
        }
        fout << "\n";
    }
    fout << " ";
    fout << "\n";
}

int main()
{
    //matrix size = row * colum
    int row = 400;
    int colum = 400;

    //create two random matrx (Mtrx1 and Mtrx2) and one multiplication matrx(multiple)
    int **Mtrx1 = new int*[row];
    int **Mtrx2 = new int*[row];

    int **Multiple = new int*[row];
    for(int i = 0; i<row; i++)
    {
        Multiple[i] = new int[colum];
    }

    //fill Mtrx1 and Mtrx2
    fillMatrix(row, colum, Mtrx1);
    fillMatrix(row, colum, Mtrx2);

    //start timer
    auto start = high_resolution_clock::now();

    //doing matrx multiplication
    for(int i = 0; i< row; i++)
    {
        for(int j = 0; j< colum; j++)
        {
            Multiple[i][j] = 0;
            for(int k = 0; k< colum; k++)
            {
                Multiple[i][j] += Mtrx1[i][k] * Mtrx2[k][j];
            }
        }
    }

    //using OpenMPI to paralell the outmost for loop
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);


    cout << "Time taken by function: "
         << duration.count() << " microseconds" << endl;
    
    //create a file called output.txt
    fout.open("output.txt");

    //write Mtrx1, Mtrx2 and Multiple to output.txt
    Print(row, colum, Mtrx1, "Matrix A: \n");
    Print(row, colum, Mtrx2, "Matrix B: \n");
    Print(row, colum, Multiple, "Matrix A*B: \n");
    fout << flush; fout.close();

    return 0;
}