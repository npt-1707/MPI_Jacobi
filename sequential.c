#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N_ROWS 20
#define N_COLS 20
#define TOLERANCE 0.0001
#define dt 0.01
#define dx 0.1
#define D 0.1

void DisplayMatrix(float *A, int row, int col);           // Display matrix
void Initialize(float *A, int row, int col);              // Initialize matrix
void Write2File(float *A, int row, int col, float tol);   // Write matrix to file
float Heat2D(float *A, float *dA, int row, int col);      // Calculate the heat changes and return the maximum change
void UpdateHeat2D(float *A, float *dA, int row, int col); // Update the heat matrix

int main(int argc, char *argv[])
{
    // Passing parameters
    int n_rows, n_cols;
    float tolerance;
    if (argc == 4)
    {
        n_rows = atoi(argv[1]);
        n_cols = atoi(argv[2]);
        tolerance = atof(argv[3]);
    }
    else if (argc == 1)
    {
        n_rows = N_ROWS;
        n_cols = N_COLS;
        tolerance = TOLERANCE;
    }
    else
    {
        printf("Usage: <name> <n_rows> <n_cols> <tolerance> or <name>\n");
        return 1;
    }
    float *A, *dA, a, north, south, west, east, maximum;
    int num_loops = 0;

    A = (float *)malloc((n_rows * n_cols) * sizeof(float));
    dA = (float *)malloc((n_rows * n_cols) * sizeof(float));

    // Initial state
    Initialize(A, n_rows, n_cols);
    Write2File(A, n_rows, n_cols, tolerance);
    clock_t start = clock();

    // Jacobi loop
    do
    {
        maximum = Heat2D(A, dA, n_rows, n_cols);
        // printf("%f", maximum);
        UpdateHeat2D(A, dA, n_rows, n_cols);
        num_loops += 1;
    } while (maximum > tolerance);
    clock_t end = clock();
    Write2File(A, n_rows, n_cols, tolerance);

    // Display result
    printf("Sequential\n");
    printf("N_rows: %d - N_cols: %d - Tolerance: %f\n", n_rows, n_cols, tolerance);
    printf("Loops: %d\n", num_loops);
    printf("Running time: %f seconds\n", (float)(end - start) / CLOCKS_PER_SEC);
    free(A);
    free(dA);
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
//==================================
void Write2File(float *A, int row, int col, float tol)
{
    char path[100];
    sprintf(path, "result/sequential_result_%d_%d_%.4f.txt", row, col, tol);
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
    fprintf(result, "\n");
    fclose(result);
}
//==================================
float Heat2D(float *A, float *dA, int row, int col)
{
    int i, j;
    float center, north, south, west, east, maximum = 0;
    for (i = 0; i < row; i++)
        for (j = 0; j < col; j++)
        {
            center = *(A + i * col + j);
            north = (i == 0) ? *(A + i * col + j) : *(A + (i - 1) * col + j);
            south = (i == row - 1) ? *(A + i * col + j) : *(A + (i + 1) * col + j);
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