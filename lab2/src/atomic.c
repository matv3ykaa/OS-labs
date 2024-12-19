#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <complex.h>
#include <stdatomic.h>
#include <time.h>

#define MAX_THREADS
#define RAND_RANGE 10

typedef double complex cplx;

typedef struct {
    _Atomic double real;
    _Atomic double imag;
} atomic_cplx;

atomic_cplx **atomic_result;

typedef struct {
    size_t start_row;
    size_t end_row;
    size_t size;
    cplx **matrix1;
    cplx **matrix2;
} ThreadArgs;

void HandleError(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    exit(EXIT_FAILURE);
}

void AllocateMatrix(cplx ***matrix, size_t size) {
    *matrix = malloc(size * sizeof(cplx *));
    if (*matrix == NULL) {
        HandleError("Ошибка выделения памяти для матрицы.\n");
    }
    for (size_t i = 0; i < size; i++) {
        (*matrix)[i] = malloc(size * sizeof(cplx));
        if ((*matrix)[i] == NULL) {
            HandleError("Ошибка выделения памяти для строки матрицы.\n");
        }
    }
}

void FreeMatrix(cplx **matrix, size_t size) {
    for (size_t i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void AllocateAtomicMatrix(atomic_cplx ***matrix, size_t size) {
    *matrix = malloc(size * sizeof(atomic_cplx *));
    if (*matrix == NULL) {
        HandleError("Ошибка выделения памяти для атомарной матрицы.\n");
    }
    for (size_t i = 0; i < size; i++) {
        (*matrix)[i] = malloc(size * sizeof(atomic_cplx));
        if ((*matrix)[i] == NULL) {
            HandleError("Ошибка выделения памяти для строки атомарной матрицы.\n");
        }
    }
}

void FreeAtomicMatrix(atomic_cplx **matrix, size_t size) {
    for (size_t i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void *MatrixMultiply(void *args) {
    ThreadArgs *data = (ThreadArgs *)args;

    for (size_t i = data->start_row; i < data->end_row; i++) {
        for (size_t j = 0; j < data->size; j++) {
            cplx sum = 0;
            for (size_t k = 0; k < data->size; k++) {
                sum += data->matrix1[i][k] * data->matrix2[k][j];
            }

            atomic_store(&atomic_result[i][j].real, creal(sum));
            atomic_store(&atomic_result[i][j].imag, cimag(sum));
        }
    }

    return NULL;
}

cplx GenerateRandomComplex() {
    double real = (rand() % (2 * RAND_RANGE + 1)) - RAND_RANGE;
    double imag = (rand() % (2 * RAND_RANGE + 1)) - RAND_RANGE;
    return real + imag * I;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        HandleError("Использование: ./atomic <количество потоков> <размер матрицы>\n");
    }

    size_t threads_count = strtoul(argv[1], NULL, 10);
    if (threads_count > MAX_THREADS) {
        HandleError("Ошибка: Превышено максимальное количество потоков.\n");
    }

    size_t matrix_size = strtoul(argv[2], NULL, 10);

    cplx **matrix1, **matrix2;

    AllocateMatrix(&matrix1, matrix_size);
    AllocateMatrix(&matrix2, matrix_size);
    AllocateAtomicMatrix(&atomic_result, matrix_size);

    srand(time(NULL));

    for (size_t i = 0; i < matrix_size; i++) {
        for (size_t j = 0; j < matrix_size; j++) {
            matrix1[i][j] = GenerateRandomComplex();
            matrix2[i][j] = GenerateRandomComplex();
        }
    }

    pthread_t threads[MAX_THREADS];
    ThreadArgs thread_args[MAX_THREADS];

    size_t rows_per_thread = matrix_size / threads_count;
    for (size_t i = 0; i < threads_count; i++) {
        thread_args[i].start_row = i * rows_per_thread;
        thread_args[i].end_row = (i == threads_count - 1) ? matrix_size : (i + 1) * rows_per_thread;
        thread_args[i].size = matrix_size;
        thread_args[i].matrix1 = matrix1;
        thread_args[i].matrix2 = matrix2;

        if (pthread_create(&threads[i], NULL, MatrixMultiply, &thread_args[i]) != 0) {
            HandleError("Ошибка создания потока.\n");
        }
    }

    for (size_t i = 0; i < threads_count; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            HandleError("Ошибка ожидания потока.\n");
        }
    }

    for (size_t i = 0; i < matrix_size; i++) {
        for (size_t j = 0; j < matrix_size; j++) {
            double real = atomic_load(&atomic_result[i][j].real);
            double imag = atomic_load(&atomic_result[i][j].imag);

            char buffer[64];
            snprintf(buffer, sizeof(buffer), "(%.2f + %.2fi) ", real, imag);
            write(STDOUT_FILENO, buffer, strlen(buffer));
        }
        write(STDOUT_FILENO, "\n", 1);
    }

    FreeMatrix(matrix1, matrix_size);
    FreeMatrix(matrix2, matrix_size);
    FreeAtomicMatrix(atomic_result, matrix_size);

    return EXIT_SUCCESS;
}
