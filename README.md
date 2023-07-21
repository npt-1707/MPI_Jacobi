# Jacobi Iterative Methods for Heat equation

### Stuctures

-   `demo` folder: consists of txt files which saves all states of heat matrix.
-   `result` folder: contains txt files which saves first and final states of heat matrix
-   `run` folder: contains compiled files from c files
-   `demo.py`: demo the output in `demo` folder to 2d heatmap
-   `3d_demo.ipynb`: demo the output in `demo` folder to 3d form. We **highly recommend** that you **upload and run** this notebook to colab and download the final demo video (the format of the output video is not supported on our local device). Please note to change the `path` variable in the notebook to the path of the txt file.
-   `sequential.c`: C program that solves sequentially the heat equation by using Jacobi iterative method.
-   `parallel.c`: C program that solves parallelly the heat equation by using Jacobi iterative method with MPI.

### Sequential program:

Compile the .c file:

```
gcc sequential.c -o run/sequential
```

Run the program

```
run/sequential <N> <N> <T>
```

Note that `N` is the size of the matrix (**Please** provide a integer divisible by 4, i.e 20, 40, 100, ...),
and `T` is the tolerance (i.e 0.01, 0.001, ...)

### Parallel program:

Compile the .c file:

```
mpicc parallel.c -o run/parallel
```

Run the program

```
mpirun -np 4 run/parallel <N> <N> <T> <mode>
```

Note that `N` is the size of the matrix (please provide a integer divided by 4),
and `T` is the tolerance (i.e, 0.01, 0.001, ...),
and `mode` is the saving mode, here we set 0 for only begining and last states are saved at "result" folder, 1 for save all states for demo in "demo" folder

### Demo the example:

run the python code:

```
python3 demo.py --path <path>
```

here the `path` is the saved path of all states above
