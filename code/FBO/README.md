Old school
----------

As soon as people noticed that GPU are higly parallel they realised that this parallelism can be used not only for graphics, but also for general purpose (GP) computing. And the GPGPU (Genereal Purpose Graphial Processing Unit) era began.

In this example we make use of OpenGL Framebuffer to define a Render Buffer where we will "render" our results to. In fragment shader we define the computational operation we would like to perform. When OpenGL pipeline runs through we can read the results of the computation from the render buffer using glReadPixels.

We also perform the same computations on the CPU and compare the performance.


#### Thanks
http://www.computer-graphics.se/gpu-computing/lab1.html
