__kernel void swap(const int size,
                      __global int* v) {
    
    for(int i = 0; i<size-1; i++){
        int count = 0;
        for(int j = 0; j<size-1;j++){
            if(v[j]>v[j+1])
            {
                int temp = v[j];
                v[j] = v[j+1];
                v[j+1] = temp;
            }
        }
        if(count == 0){
            break;
        }
    }

}
