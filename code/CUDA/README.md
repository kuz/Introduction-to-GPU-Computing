CUDA
----

CUDA is a GPGPU standart developed and maintained by nVidia.<br>
Here is the list of supported hardware: https://developer.nvidia.com/cuda-gpus

CUDA is arguable more user-friendly and has better shaped developer tools and support.

### Instructions

To run this example you will need
* CUDA-copmatible graphics card
* Install CUDA SDK

Once these conditions are satisfied,
<code>
nvcc -o square square.cu
./square
</code>
