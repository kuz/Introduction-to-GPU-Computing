//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>

// Simple compute kernel which computes the square of an input array
const char *KernelSource = "\n" \
"__kernel void square(                                                  \n" \
"   __global float* input,                                              \n" \
"   __global float* output,                                             \n" \
"   const unsigned long data_size)                                      \n" \
"{                                                                      \n" \
"   int i = get_global_id(0);                                           \n" \
"   if(i < data_size)                                                   \n" \
"       output[i] = sqrt(input[i]);                                     \n" \
"}                                                                      \n" \
"\n";

#define DATA_SIZE 100000000

int main(int argc, char** argv)
{
    int err;                            // error code returned from api calls
    
    unsigned long data_size = DATA_SIZE;
    
    static float data[DATA_SIZE];              // original data set given to device
    static float gpu_results[DATA_SIZE];           // results returned from device
    static float cpu_results[DATA_SIZE];           // results returned from device

    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation

    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    
    cl_mem input;                       // device memory used for the input array
    cl_mem output;                      // device memory used for the output array
    
    // Fill our data set with random float values
    for(long i = 0; i < data_size; i++) {
        data[i] = (float)(int)rand();
    }
    
    // Track time needed for all GPU operations
    clock_t start = clock();
    
    // Connect to a compute device
    int gpu = 1;
    err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
  
    // Create a compute context
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);

    // Create a command commands
    commands = clCreateCommandQueue(context, device_id, 0, &err);

    // Create the compute program from the source buffer
    program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);

    // Build the program executable
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);

    // Create the compute kernel in the program we wish to run
    kernel = clCreateKernel(program, "square", &err);

    // Create the input and output arrays in device memory for our calculation
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * data_size, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * data_size, NULL, NULL);
    
    // Write our data set into the input array in device memory
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * data_size, data, 0, NULL, NULL);

    // Set the arguments to our compute kernel
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &data_size);

    // Get the maximum work group size for executing the kernel on the device
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    global = data_size;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);

    // Wait for the command commands to get serviced before reading back results
    clFinish(commands);

    // Read back the results from the device to verify the output
    err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, sizeof(float) * data_size, gpu_results, 0, NULL, NULL );
    
    clock_t end = clock();
    float gpu_sec = (float)(end - start) / CLOCKS_PER_SEC;
    
    
    // Same on CPU
    start = clock();
    for(long i = 0; i < data_size; i++) {
        cpu_results[i] = sqrt(data[i]);
    }
    end = clock();
    float cpu_sec = (float)(end - start) / CLOCKS_PER_SEC;
    
    // Print a brief summary detailing the results
    printf("CPU time %f\n", cpu_sec);
    printf("GPU time %f\n", gpu_sec);
    
    // Shutdown and cleanup
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}

