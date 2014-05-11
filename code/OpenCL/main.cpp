#include <iostream>
#include <cassert>

#ifdef __APPLE__
    #include <OpenCL/opencl.h>
#else
    #include <CL/cl.h>
#endif

using std::cout;
using std::endl;

void vector_add_cpu(const float* const src_a,
                    const float* const src_b,
                    float* const res,
                    const int size)
{
    for (int i = 0; i < size; i++) {
        res[i] = src_a[i] + src_b[i];
    }
}

int main() {
    const size_t size = 12345678;
    const size_t mem_size = sizeof(float)*size;

    // OpenCL Environment variables
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_int error = CL_SUCCESS;

    // Initializing the basic OpenCL environment
    error = oclGetPlatformID(&platform);
    assert(error == CL_SUCCESS);
    error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    assert(error == CL_SUCCESS);
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &error);
    assert(error == CL_SUCCESS);
    queue = clCreateCommandQueue(context, device, NULL, &error);
    assert(error == CL_SUCCESS);

    // Initializing host memory
    float* src_a_h = new float[size];
    float* src_b_h = new float[size];
    float* res_h = new float[size];

    for (int i = 0; i < size; i++) {
        src_a_h[i] = src_b_h[i] = 2.0f*i;
    }

    // Initializing device memory
    cl_mem src_a_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_a_h, &error);
    assert(error == CL_SUCCESS);
    cl_mem src_b_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, mem_size, src_b_h, &error);
    assert(error == CL_SUCCESS);
    cl_mem res_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size, NULL, &error);
    assert(error == CL_SUCCESS);

    // Creates the program
    // Uses NVIDIA helper functions to get the code string and it's size (in bytes)
    size_t src_size = 0;
    const char* path = shrFindFilePath("vector_add_gpu.cu", NULL);
    const char* source = oclLoadProgSource(path, "", &src_size);
    cl_program program = clCreateProgramWithSource(context, 1, &source, &src_size, &error);
    assert(error == CL_SUCCESS);

    // Builds the program
    error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    assert(error == CL_SUCCESS);

    // Shows the log
    char* build_log;
    size_t log_size;
    // First call to know the proper size
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
    build_log = new char[log_size+1];
    // Second call to get the log
    clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
    build_log[log_size] = '\0';
    cout << build_log << endl;
    delete[] build_log;

    // 'Extracting' the kernel
    cl_kernel vector_add_k = clCreateKernel(program, "vector_add_gpu", &error);
    assert(error == CL_SUCCESS);
    
    // Enqueuing parameters
    error = clSetKernelArg(vector_add_k, 0, sizeof(cl_mem), &src_a_d);
    error |= clSetKernelArg(vector_add_k, 1, sizeof(cl_mem), &src_b_d);
    error |= clSetKernelArg(vector_add_k, 2, sizeof(cl_mem), &res_d);
    error |= clSetKernelArg(vector_add_k, 3, sizeof(size_t), &size);
    assert(error == CL_SUCCESS);

    // Launching kernel
    const size_t local_ws = 512;    // Number of work-items per work-group
    // shrRoundUp returns the smallest multiple of local_ws bigger than size
    const size_t global_ws = shrRoundUp(local_ws, size);    // Total number of work-items
    error = clEnqueueNDRangeKernel(queue, vector_add_k, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
    assert(error == CL_SUCCESS);

    // Reading back
    float* check = new float[size];
    clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, check, 0, NULL, NULL);

    // Checking with the CPU results;
    vector_add_cpu(src_a_h, src_b_h, res_h, size);
    for (int i = 0; i < size; i++)
        assert(check[i] == res_h[i]);

    cout << "\nCongratulations, it's working!" << endl;
    
    // Cleaning up
    delete[] src_a_h;
    delete[] src_b_h;
    delete[] res_h;
    delete[] check;
    clReleaseKernel(vector_add_k);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    clReleaseMemObject(src_a_d);
    clReleaseMemObject(src_b_d);
    clReleaseMemObject(res_d);

    return 0;
}