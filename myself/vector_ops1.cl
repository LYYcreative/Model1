__kernel void square_magnitude(const int size,
                      __global int* v) {
    
    // Thread identifiers
    const int globalIndex = get_global_id(0); // Row ID of C (0..M)   
 
    //uncomment to see the index each PE works on
    //printf("Kernel process index :(%d)\n ", globalIndex);

    v[globalIndex] = v[globalIndex] * v[globalIndex];
}


__kernel void add_vector(const int size,
                      const __global int* v1,const __global int* v2,__global int* v3) {
    
    // Thread identifiers
    const int globalIndex = get_global_id(0);    
 
    //uncomment to see the index each PE works on
    //printf("Kernel process index :(%d)\n ", globalIndex);

    v3[globalIndex] = v2[globalIndex] + v1[globalIndex];
}
