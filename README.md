# Jacobi Iterative Methods for Heat equation

### Sequential program:

Compile the .c file:

```
gcc sequential.c -o run/sequential
```

Run the program

```
run/sequential <N> <N> <T>
```

Note that `N` is the size of the matrix (please provide a integer divided by 4),
and `T` is the tolerance (i.e, 0.01, 0.001, ...)

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
and `mode` is the saving mode, - here we set 0 for only begining and last states are saved at "result" folder - 1 for save all states for demo in "demo" folder

### Demo the example:

run the python code:

```
python3 demo.py --path <path>
```

here the `path` is the saved path of all states above
