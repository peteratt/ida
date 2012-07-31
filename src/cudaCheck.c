/* Author:  Corentin Debains
 * Email:   cdebains@iit.edu
 *
 *
 * CUDA Capabilities Checking function
 */
 
#include "../inc/globals.h"
#include <stdio.h>
#include <cuda_runtime.h>

int CUDACapableCheck() {
    int deviceCount, device;
    int gpuDeviceCount = 0;
    struct cudaDeviceProp properties;
    cudaError_t cudaResultCode = cudaGetDeviceCount(&deviceCount);
    if (cudaResultCode != cudaSuccess) 
        deviceCount = 0;
    /* machines with no GPUs can still report one emulation device */
    for (device = 0; device < deviceCount; ++device) {
        cudaGetDeviceProperties(&properties, device);
        if (properties.major != 9999) /* 9999 means emulation only */
            ++gpuDeviceCount;
    }
    dbgprintf("%d GPU CUDA device(s) found\n", gpuDeviceCount);

    /* don't just return the number of gpus, because other runtime cuda
       errors can also yield non-zero return values */
    if (gpuDeviceCount > 0)
        return 0; /* success */
    else
        return 1; /* failure */
}


