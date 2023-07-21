#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

#define N_ROWS 20
#define N_COLS 20
#define dt 0.01
#define dx 0.1
#define D 0.1
#define TOLERANCE 0.001

void DisplayMatrix(float *A, int row, int col);
void Initialize(float *A, int row, int col);
void Write2File(float *A, int row, int col, float tol, int mode);
float Heat2D(float *A, float *dA, float *Top, float *Bottom, int row, int col);
void UpdateHeat2D(float *A, float *dA, int row, int col);
float max(float *A, int n);

int main(int argc, char *argv[])
{
    int n_rows, n_cols, mode;
    float tolerance;
    if (argc == 5)
    {
        n_rows = atoi(argv[1]);
        n_cols = atoi(argv[2]);
        tolerance = atof(argv[3]);
        mode = atoi(argv[4]);
    }
    else if (argc == 1)
    {
        n_rows = N_ROWS;
        n_cols = N_COLS;
        tolerance = TOLERANCE;
        mode = 0;
    }
    else
    {
        printf("Usage: <name> <n_rows> <n_cols> <tolerance> <mode> or <name>\n");
        return 1;
    }

    float *A, *dA, *subA, *subdA, *Top, *Bottom, *M;
    float maximum, global_maximum, start, end;
    int NP, rank, size, num_loops = 0;
    bool running = true;
    A = (float *)malloc((n_rows * n_cols) * sizeof(float));
    dA = (float *)malloc((n_rows * n_cols) * sizeof(float));
    MPI_Status status;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &NP);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    size = n_rows / NP;
    subA = (float *)malloc((size * n_cols) * sizeof(float));
    subdA = (float *)malloc((size * n_cols) * sizeof(float));
    Top = (float *)malloc(n_cols * sizeof(float));
    Bottom = (float *)malloc(n_cols * sizeof(float));
    // M = (float *)malloc(NP * sizeof(float));
    if (rank == 0)
    {
        Initialize(A, n_rows, n_cols);
        Write2File(A, n_rows, n_cols, tolerance, 0);
        if (mode == 1)
        {
            Write2File(A, n_rows, n_cols, tolerance, 1);
        }
    }
    MPI_Barrier(MPI_COMM_WORLD);
    start = MPI_Wtime();
    MPI_Scatter(A, size * n_cols, MPI_FLOAT,
                subA, size * n_cols, MPI_FLOAT, 0,
                MPI_COMM_WORLD);
    // if (rank == 0)
    // {
    //     for (int dest_rank = 1; dest_rank < NP; dest_rank++)
    //     {
    //         MPI_Send(&A[dest_rank * size * n_cols], size * n_cols, MPI_FLOAT, dest_rank, 0, MPI_COMM_WORLD);
    //     }
    //     // Root process keeps its part of the data
    //     memcpy(subA, &A[0], size * n_cols * sizeof(float));
    // }
    // else
    // {
    //     MPI_Recv(subA, size * n_cols, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // }
    // MPI_Barrier(MPI_COMM_WORLD);

    do
    {
        if (rank == 0)
        {
            for (int j = 0; j < n_cols; j++)
                *(Top + j) = *(subA + 0 * n_cols + j);
            MPI_Send(subA + (size - 1) * n_cols, n_cols, MPI_FLOAT, rank + 1, rank, MPI_COMM_WORLD);
        }
        else if (rank == NP - 1)
        {
            MPI_Recv(Top, n_cols, MPI_FLOAT, rank - 1, rank - 1, MPI_COMM_WORLD, &status);
        }
        else
        {
            MPI_Send(subA + (size - 1) * n_cols, n_cols, MPI_FLOAT, rank + 1, rank, MPI_COMM_WORLD);
            MPI_Recv(Top, n_cols, MPI_FLOAT, rank - 1, rank - 1, MPI_COMM_WORLD, &status);
        }
        //
        if (rank == NP - 1)
        {
            for (int j = 0; j < n_cols; j++)
                *(Bottom + j) = *(subA + (size - 1) * n_cols + j);
            MPI_Send(subA, n_cols, MPI_FLOAT, rank - 1, rank, MPI_COMM_WORLD);
        }
        else if (rank == 0)
        {
            MPI_Recv(Bottom, n_cols, MPI_FLOAT, rank + 1, rank + 1, MPI_COMM_WORLD, &status);
        }
        else
        {
            MPI_Send(subA, n_cols, MPI_FLOAT, rank - 1, rank, MPI_COMM_WORLD);
            MPI_Recv(Bottom, n_cols, MPI_FLOAT, rank + 1, rank + 1, MPI_COMM_WORLD, &status);
        }

        maximum = Heat2D(subA, subdA, Top, Bottom, size, n_cols);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Allreduce(&maximum, &global_maximum, 1, MPI_FLOAT, MPI_MAX, MPI_COMM_WORLD);
        // *(M + rank) = maximum;
        // for (int i = 0; i < NP; i++)
        // {
        //     if (rank != i)
        //     {
        //         MPI_Send(&maximum, 1, MPI_FLOAT, i, rank, MPI_COMM_WORLD);
        //         MPI_Recv(M + i, 1, MPI_FLOAT, i, i, MPI_COMM_WORLD, &status);
        //     }
        // }
        // global_maximum = max(M, NP);
        UpdateHeat2D(subA, subdA, size, n_cols);
        num_loops++;
        MPI_Barrier(MPI_COMM_WORLD);
        if (mode == 1)
        {
            MPI_Gather(subA, size * n_cols, MPI_FLOAT, A, size * n_cols, MPI_FLOAT, 0, MPI_COMM_WORLD);
            if (rank == 0)
            {
                Write2File(A, n_rows, n_cols, tolerance, 1);
            }
        }
    } while (global_maximum > tolerance);

    MPI_Gather(subA, size * n_cols, MPI_FLOAT, A, size * n_cols, MPI_FLOAT, 0, MPI_COMM_WORLD);
    end = MPI_Wtime();
    if (rank == 0)
    {   
        Write2File(A, n_rows, n_cols, tolerance, 0);
        printf("Parallel\n");
        printf("N_rows: %d - N_cols: %d - Tolerance: %f\n", n_rows, n_cols, tolerance);
        printf("Loops: %d\n", num_loops);
        printf("Running time: %f\n", end - start);
    }
    free(A);
    free(dA);
    free(subA);
    free(subdA);
    free(Top);
    free(Bottom);
    free(M);
    MPI_Finalize();
    return 0;
}

//==================================
void DisplayMatrix(float *A, int row, int col)
{
    int i, j;
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
            printf("  %f", *(A + i * col + j));
        printf("\n");
    }
}
//==================================
void Initialize(float *A, int row, int col)
{
    int i, j;
    for (i = 0; i < row; i++)
        for (j = 0; j < col; j++)
        {
            if (i >= (row * 0.4) && i < (row * 0.6) && j >= (col * 0.4) && j < (col * 0.6))
                *(A + i * col + j) = 80.0;
            else
                *(A + i * col + j) = 25.0;
        }
}
// void Initialize(float *A, int row, int col)
// {
//     int i, j;
//     float center_x = (row - 1) / 2;                                 // x-coordinate of the center of the matrix
//     float center_y = (col - 1) / 2;                                 // y-coordinate of the center of the matrix
//     float radius = (row < col) ? (row - 1) * 0.1 : (col - 1) * 0.1; // Adjust the circle radius based on the smaller dimension of the matrix

//     for (i = 0; i < row; i++)
//     {
//         for (j = 0; j < col; j++)
//         {
//             // Calculate the distance from the center of the matrix to the current position (i, j)
//             float distance = sqrt((i - center_x) * (i - center_x) + (j - center_y) * (j - center_y));

//             if (distance <= radius)
//             {
//                 *(A + i * col + j) = 80.0; // Inside the circle
//             }
//             else
//             {
//                 *(A + i * col + j) = 25.0; // Outside the circle
//             }
//         }
//     }
// }

//==================================
void Write2File(float *A, int row, int col, float tol, int mode)
{
    char *dir = (mode == 0) ? "result" : "demo";
    char path[100];
    sprintf(path, "%s/parallel_result_%d_%d_%.4f.txt", dir, row, col, tol);
    FILE *result = fopen(path, "a");
    int i, j;
    for (i = 0; i < row; i++)
    {
        for (j = 0; j < col; j++)
        {
            fprintf(result, "%lf\t", *(A + i * col + j));
        }
        fprintf(result, "\n");
    }
    if (mode == 0)
    {
        fprintf(result, "\n");
    }
    fclose(result);
}
//==================================
float Heat2D(float *A, float *dA, float *Top, float *Bottom, int row, int col)
{
    int i, j;
    float center, north, south, west, east, maximum = 0;
    for (i = 0; i < row; i++)
        for (j = 0; j < col; j++)
        {
            center = *(A + i * col + j);
            north = (i == 0) ? *(Top + j) : *(A + (i - 1) * col + j);
            south = (i == row - 1) ? *(Bottom + j) : *(A + (i + 1) * col + j);
            west = (j == 0) ? *(A + i * col + j) : *(A + i * col + (j - 1));
            east = (j == col - 1) ? *(A + i * col + j) : *(A + i * col + (j + 1));
            *(dA + i * col + j) = D * (north + south + west + east - 4 * center) / (dx * dx);
            if (*(dA + i * col + j) > maximum)
            {
                maximum = *(dA + i * col + j);
            }
        }

    return maximum;
}
//==================================
void UpdateHeat2D(float *A, float *dA, int row, int col)
{
    int i, j;
    for (i = 0; i < row; i++)
        for (j = 0; j < col; j++)
        {
            *(A + i * col + j) += *(dA + i * col + j) * dt;
        }
}
//==================================
float max(float *A, int n)
{
    float maximum = *A;
    for (int i = 1; i < n; i++)
    {
        if (maximum < *(A + i))
        {
            maximum = *(A + i);
        }
    }
    return maximum;
}