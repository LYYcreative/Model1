__kernel void add_matrices(const int M, const int N, const int K,
                      const __global int* A,
                      const __global int* B,
                      __global int* C) {
    
    // Thread identifiers
    const int globalRow = get_global_id(0); // Row ID of C (0..M)
    const int globalCol = get_global_id(1); // Col ID of C (0..N)
 
    printf("Kernel print::(%d,%d)\n ", globalRow, globalCol);
    C[globalCol*M + globalRow] = A[globalCol*M + globalRow] + B[globalCol*M + globalRow];
    
}


__kernel void multiply_matrices(const int M, const int N, const int K,
                      const __global int* A,
                      const __global int* B,
                      __global int* C) {
    
    
    int sum;
    int colindex = 0;
    int rowindex = 0;
    for(int j = 0; j<M*N;j++)
    {
        
        for(int i = 0; i<M;i++)
        {
            sum +=  A[i+rowindex] * B[M*i+colindex];

        }
        colindex += 1;
        if(colindex > M-1){
            colindex = 0;
            rowindex += M;
        }
        if(rowindex>(M-1)*N){
            rowindex = 0;
        }

        C[j] = sum;
        sum = 0;
        
    }
    
}




/*
    //printf("(%d,%d)\n ", globalRow, globalCol);
    // Compute a single element (loop over K)
    int acc = 0.0f;
    for (int k=0; k<K; k++) {
        acc += B[k*M + globalRow] * A[globalCol*K + k];
     //   printf("(%d,%d), values = (%d, %d)\n ", k*M + globalRow, globalCol*K + k, A[k*M + globalRow] , B[globalCol*K + k]);
    }
*/