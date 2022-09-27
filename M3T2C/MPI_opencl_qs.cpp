#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <CL/cl.h>
#include <iostream>
#include <time.h>
#include <chrono>
using namespace std::chrono;
using namespace std;

#define PRINT 1

int SZ = 4;
int *v;

//ToDo: Add comment (what is the purpose of this variable)
cl_mem bufV;

//ToDo: Add comment (what is the purpose of this variable)
cl_device_id device_id;
//ToDo: Add comment (what is the purpose of this variable)
cl_context context;
//ToDo: Add comment (what is the purpose of this variable)
cl_program program;
//ToDo: Add comment (what is the purpose of this variable)
cl_kernel kernel;
//ToDo: Add comment (what is the purpose of this variable)
cl_command_queue queue;
cl_event event = NULL;

int err;

//ToDo: Add comment (what is the purpose of this function)
cl_device_id create_device();
//ToDo: Add comment (what is the purpose of this function)
void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname);
//ToDo: Add comment (what is the purpose of this function)
cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename);
//ToDo: Add comment (what is the purpose of this function)
void setup_kernel_memory();
//ToDo: Add comment (what is the purpose of this function)
void copy_kernel_args();
//ToDo: Add comment (what is the purpose of this function)
void free_memory();

void init(int *&A, int size);
void print(int *A, int size);
void Merge(int left, int right, int *A);

int main(int argc, char **argv)
{
    int Size = 16;

    int *v1, *result;

    int numtasks, rank, name_len, tag = 1;
    int res;
    MPI_Status status;
    // Initialize the MPI environment
    MPI_Init(&argc,&argv);

    // Get the number of tasks/process
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);

    // Get the rank
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    v = (int *) malloc(SZ * sizeof(int *));

    if(rank == 0)
    {
        result = (int *) malloc(Size * sizeof(int *));
        if (argc > 1)
            SZ = atoi(argv[1]);

        init(v1, Size);
        printf("Origenal Vector: ");
        print(v1, Size);
        printf("\n");
    }
    


    //ToDo: Add comment (what is the purpose of this variable)
    

    //initial vector
    auto start = high_resolution_clock::now();
    MPI_Scatter(v1, SZ, MPI_INT, v, SZ, MPI_INT, 0,MPI_COMM_WORLD);
    size_t global[1] = {(size_t)SZ};

    setup_openCL_device_context_queue_kernel((char *)"./vector_ops_1.cl", (char *)"swap");

    setup_kernel_memory();
    copy_kernel_args();

    
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global, NULL, 0, NULL, &event);
    clWaitForEvents(1, &event);

    //ToDo: Add comment (what is the purpose of this function? What are its arguments?)
    clEnqueueReadBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);

    MPI_Gather(v, SZ, MPI_INT, result, SZ, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);

    if(rank == 0)
    {

        Merge(0,(SZ*2)-1,result);
        Merge(SZ*2,Size-1,result);
        Merge(0,Size-1,result);
        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop - start);

        printf("Sorted vector: ");
        print(result, Size);

        cout << "Time taken by function: "
         << duration.count() << " microseconds"
         << endl;

        free_memory();
    }
    
    MPI_Finalize();
    //frees memory for device, kernel, queue, etc.
    //you will need to modify this to free your own buffers
    
}

void Merge(int left, int right, int *A)
{
    int mid = (left+right)/2;
    
    //compare values in each sub-vector and replace them to right place
    int i=left, j=mid+1, k=0; 
    int *temp = (int *) malloc((right-left+1) * sizeof(int *));
    
    while(i<=mid&&j<=right)
    {
        if(A[i]<=A[j]) 
            temp[k++]=A[i++];
        else
            temp[k++]=A[j++];
    }
    while(i<=mid)
        temp[k++]=A[i++];
    while(j<=right)
        temp[k++]=A[j++];
    for(i=left,k=0;i<=right;i++,k++)
		A[i]=temp[k];
    delete []temp;

    return;

}

void init(int *&A, int size)
{
    A = (int *)malloc(sizeof(int) * size);

    int num = 0;
	for(int i = 0; i < size; i++)
	{
		num = i;
		// to create a set of numbers that is neither increasing nor decrementing
		if(num %2 == 0)
		{
			
			A[i] = num -1;
		}
		else
		{
			A[i] = num+1;
		}
	}
}

void print(int *A, int size)
{
    for(int i = 0; i< size;i++)
    {
        printf("%d",A[i]);
        printf(" ");
        
    }
    printf("\n");
}

void free_memory()
{
    //free the buffers
    clReleaseMemObject(bufV);

    //free opencl objects
    clReleaseKernel(kernel);
    clReleaseCommandQueue(queue);
    clReleaseProgram(program);
    clReleaseContext(context);

    free(v);
}


void copy_kernel_args()
{
    //ToDo: Add comment (what is the purpose of clSetKernelArg function? What are its arguments?)
    clSetKernelArg(kernel, 0, sizeof(int), (void *)&SZ);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&bufV);

    if (err < 0)
    {
        perror("Couldn't create a kernel argument");
        printf("error = %d", err);
        exit(1);
    }
}

void setup_kernel_memory()
{
    //ToDo: Add comment (what is the purpose of clCreateBuffer function? What are its arguments?) 
    //The second parameter of the clCreateBuffer is cl_mem_flags flags. Check the OpenCL documention to find out what is it's purpose and read the List of supported memory flag values 
    bufV = clCreateBuffer(context, CL_MEM_READ_WRITE, SZ * sizeof(int), NULL, NULL);

    // Copy matrices to the GPU
    clEnqueueWriteBuffer(queue, bufV, CL_TRUE, 0, SZ * sizeof(int), &v[0], 0, NULL, NULL);
}

void setup_openCL_device_context_queue_kernel(char *filename, char *kernelname)
{
    device_id = create_device();
    cl_int err;

    //ToDo: Add comment (what is the purpose of clCreateBuffer function?)
    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
    if (err < 0)
    {
        perror("Couldn't create a context");
        exit(1);
    }

    program = build_program(context, device_id, filename);

    //ToDo: Add comment (what is the purpose of clCreateCommandQueueWithProperties function?)
    queue = clCreateCommandQueueWithProperties(context, device_id, 0, &err);
    if (err < 0)
    {
        perror("Couldn't create a command queue");
        exit(1);
    };


    kernel = clCreateKernel(program, kernelname, &err);
    if (err < 0)
    {
        perror("Couldn't create a kernel");
        printf("error =%d", err);
        exit(1);
    };
}

cl_program build_program(cl_context ctx, cl_device_id dev, const char *filename)
{

    cl_program program;
    FILE *program_handle;
    char *program_buffer, *program_log;
    size_t program_size, log_size;

    /* Read program file and place content into buffer */
    program_handle = fopen(filename, "r");
    if (program_handle == NULL)
    {
        perror("Couldn't find the program file");
        exit(1);
    }
    fseek(program_handle, 0, SEEK_END);
    program_size = ftell(program_handle);
    rewind(program_handle);
    program_buffer = (char *)malloc(program_size + 1);
    program_buffer[program_size] = '\0';
    fread(program_buffer, sizeof(char), program_size, program_handle);
    fclose(program_handle);

    //ToDo: Add comment (what is the purpose of clCreateProgramWithSource function? What are its arguments?)
    program = clCreateProgramWithSource(ctx, 1,
                                        (const char **)&program_buffer, &program_size, &err);
    if (err < 0)
    {
        perror("Couldn't create the program");
        exit(1);
    }
    free(program_buffer);

    /* Build program 
   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if (err < 0)
    {

        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              0, NULL, &log_size);
        program_log = (char *)malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG,
                              log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    return program;
}

cl_device_id create_device() {

   cl_platform_id platform;
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(1, &platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   // Access a device
   // GPU
   err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      // CPU
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}
