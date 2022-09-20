#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <chrono>
#include <math.h>
#include <mpi.h>
#include <CL/cl.hpp>

#define N 100

using namespace std;

// NOTE: I consulted several online resources in order to wrap my head around OpenCL, including:
//         - https://github.com/Dakkers/OpenCL-examples
//         - https://stackoverflow.com/questions/35442327/2d-array-as-opencl-kernel-argument
//         - https://en.wikipedia.org/wiki/OpenCL

class Timer {
public:
    Timer() : beg_(clock_::now()) {}
    void reset() { beg_ = clock_::now(); }
    double elapsed() const {
        return chrono::duration_cast<second_>
            (clock_::now() - beg_).count(); }

private:
    typedef chrono::high_resolution_clock clock_;
    typedef chrono::duration<double, ratio<1> > second_;
    chrono::time_point<clock_> beg_;
};

// Populates the given matrix with random numbers.
void populateMatrix(int a[N*N]) {
    for (int i=0; i<N*N; i++) {
        a[i] = rand() % 100;
    }
}

// Partially multiplies matrices a and b from offsets start to stop. Results are
// sent to the zeroth process.
void partiallyMultiplyMatrices(int start, int end, int a[N*N], int b[N*N]) {
    vector<cl::Platform> all_platforms;

    cl::Platform::get(&all_platforms);
    cl::Platform default_platform = all_platforms[0];
 
    vector<cl::Device> all_devices;
    default_platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);

    cl::Device default_device = all_devices[1]; // GPU
 
    cl::Context context({default_device});
    cl::Program::Sources sources;

    stringstream kernel_code;
    kernel_code <<
        "void kernel matrix_multiply(global const int* A, global const int* B, global int* Results, "
        "                            global const int* Start, global const int* End) {"
        "    int rowSize, ID, threads, start, end, "
        "        ratio, from, stop, row, col;"
        ""
        "    rowSize = " << N << ";"
        "    ID = get_global_id(0);"
        "    threads = get_global_size(0);"
        "    start = Start[0];"
        "    end = End[0];"
        "    ratio = ((end - start) / threads);"  // number of elements for each thread
        "    from = (ratio * ID);"
        "    stop = (ID == threads - 1) ?" // Last thread picks up remaining work
        "      (end - start) - 1 :"
        "      from + ratio;"
        ""
        "    for (int i=from; i<=stop; i++) {"
        "        row = (start + i) / rowSize;"
        "        col = (start + i) % rowSize;"
        "        Results[i] = 0;"
        ""
        "        for (int j=0; j<rowSize; j++) {"
        "            Results[i] += A[(row * rowSize) + j] * B[(j * rowSize) + col];"
        "        }"
        "    }"
        "}";
    string kernel = kernel_code.str();

    sources.push_back({kernel.c_str(), kernel.length()});

    cl::Program program(context, sources);
    if (program.build({default_device}) != CL_SUCCESS) {
        cout << "Error: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << endl;
        exit(1);
    }

    int aStart[1] = { start };
    int aEnd[1] = { end };

    // Setup buffers and write local data to GPU
    cl::Buffer buffer_A(context, CL_MEM_READ_WRITE, sizeof(int) * N * N);
    cl::Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(int) * N * N);
    cl::Buffer buffer_results(context, CL_MEM_READ_WRITE, sizeof(int) * (end - start));
    cl::Buffer buffer_start(context, CL_MEM_READ_ONLY,  sizeof(int));
    cl::Buffer buffer_end(context, CL_MEM_READ_ONLY,  sizeof(int));

    cl::CommandQueue queue(context, default_device);

    queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(int) * N * N, a);
    queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(int) * N * N, b);
    queue.enqueueWriteBuffer(buffer_start, CL_TRUE, 0, sizeof(int), aStart);
    queue.enqueueWriteBuffer(buffer_end, CL_TRUE, 0, sizeof(int), aEnd);

    // Execute kernel code in GPU
    cl::Kernel matrix_multiply(program, "matrix_multiply");
    matrix_multiply.setArg(0, buffer_A);
    matrix_multiply.setArg(1, buffer_B);
    matrix_multiply.setArg(2, buffer_results);
    matrix_multiply.setArg(3, buffer_start);
    matrix_multiply.setArg(4, buffer_end);
    queue.enqueueNDRangeKernel(matrix_multiply, cl::NullRange, cl::NDRange(10), cl::NullRange);

    // Read results from GPU
    int results[end - start];
    queue.enqueueReadBuffer(buffer_results, CL_TRUE, 0, sizeof(int) * (end - start), results);

    // Send results back to master process
    MPI_Send(results, end - start, MPI_INT, 0, 0, MPI_COMM_WORLD);
}

// Writes the given matrix to file.
void persistToFile(string path, int m[N*N]) {
    ofstream outfile;
    outfile.open(path);

    for (int i=0; i<N*N; i++) {
        outfile << m[i] << " ";

        if (i % N == N-1) {
            outfile << endl;
        }
    }

    outfile.close();
}

int main(int argc, char** argv) {
    int rank, size, start, end;
    int matrixSize = N * N;
    int* a = new int[matrixSize];
    int* b = new int[matrixSize];
    int* c = new int[matrixSize];

    populateMatrix(a);
    populateMatrix(b);

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    Timer tmr;

    int chunkSize = floor(matrixSize / (size - 1));

    // Slave processes compute their own portion of the matrix and send results to the master.
    if (rank > 0) {
        start = (rank - 1) * chunkSize;
        end = (rank == size - 1) ?
          matrixSize :
          start + chunkSize;

        partiallyMultiplyMatrices(start, end, a, b);

    // Master process waits for results from slaves and mutates result matrix.
    } else {
        MPI_Status status;
        int start, count;
        int results[chunkSize + 1];

        for (int rank=1; rank<size; rank++) {
            MPI_Recv(results, chunkSize + 1, MPI_INT, rank, 0, MPI_COMM_WORLD, &status);
            MPI_Get_count(&status, MPI_INT, &count);

            start = (rank - 1) * chunkSize;

            for (int i=start; i<(start + count); i++) {
                c[i] = results[i - start];
            }
        }

        double t = tmr.elapsed();

        persistToFile("result.txt", c);

        cout << "Elapsed time: " << (t * 1000) << endl;
    }

    delete [] a;
    delete [] b;
    delete [] c;

    MPI_Finalize();

    return 0;
}
