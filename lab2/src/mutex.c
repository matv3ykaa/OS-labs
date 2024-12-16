#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <string.h>
#include <errno.h>

#define MAX_ARRAYS 100
#define MAX_SIZE 1000
#define BUFFER_SIZE 1024

typedef struct {
    int** arrays;
    int* result;
    int start;
    int end;
    int length;
    pthread_mutex_t* mutex;
} ThreadData;

void handle_error(const char* msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    write(STDERR_FILENO, "\n", 1);
    exit(EXIT_FAILURE);
}

void* sum_arrays(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    for (int i = data->start; i < data->end; i++) {
        for (int j = 0; j < data->length; j++) {
            pthread_mutex_lock(data->mutex);
            data->result[j] += data->arrays[i][j];
            pthread_mutex_unlock(data->mutex);
        }
    }

    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        const char* usage = "Usage: <number of arrays> <array length> <number of threads>\n";
        write(STDERR_FILENO, usage, strlen(usage));
        return EXIT_FAILURE;
    }

    int num_arrays = atoi(argv[1]);
    int array_length = atoi(argv[2]);
    int num_threads = atoi(argv[3]);

    if (num_arrays <= 0 || array_length <= 0 || num_threads <= 0) {
        handle_error("Error: All input values must be positive integers.");
    }

    if (num_arrays > MAX_ARRAYS || array_length > MAX_SIZE) {
        handle_error("Error: Exceeded maximum array or size limit.");
    }

    int max_threads = get_nprocs();
    if (num_threads > max_threads) {
        char buffer[BUFFER_SIZE];
        int len = snprintf(buffer, BUFFER_SIZE, 
            "Error: Number of threads (%d) exceeds the number of logical cores (%d).\n", 
            num_threads, max_threads);
        write(STDERR_FILENO, buffer, len);
        return EXIT_FAILURE;
    }

    int** arrays = (int**)malloc(num_arrays * sizeof(int*));
    if (arrays == NULL) handle_error("Error allocating memory for arrays.");

    for (int i = 0; i < num_arrays; i++) {
        arrays[i] = (int*)malloc(array_length * sizeof(int));
        if (arrays[i] == NULL) handle_error("Error allocating memory for an array.");
        for (int j = 0; j < array_length; j++) {
            arrays[i][j] = rand() % 10;
        }
    }

    int* result = (int*)calloc(array_length, sizeof(int));
    if (result == NULL) handle_error("Error allocating memory for result.");

    pthread_t threads[num_threads];
    pthread_mutex_t mutex;

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        handle_error("Error initializing mutex.");
    }

    ThreadData thread_data[num_threads];
    int chunk_size = (num_arrays + num_threads - 1) / num_threads;

    for (int i = 0; i < num_threads; i++) {
        thread_data[i].arrays = arrays;
        thread_data[i].result = result;
        thread_data[i].start = i * chunk_size;
        thread_data[i].end = (i + 1) * chunk_size > num_arrays ? num_arrays : (i + 1) * chunk_size;
        thread_data[i].length = array_length;
        thread_data[i].mutex = &mutex;

        if (pthread_create(&threads[i], NULL, sum_arrays, &thread_data[i]) != 0) {
            handle_error("Error creating thread.");
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            handle_error("Error joining thread.");
        }
    }

    char buffer[BUFFER_SIZE];
    int len = 0;
    len += snprintf(buffer + len, BUFFER_SIZE - len, "Result array:\n");
    for (int i = 0; i < array_length; i++) {
        len += snprintf(buffer + len, BUFFER_SIZE - len, "%d ", result[i]);
    }
    len += snprintf(buffer + len, BUFFER_SIZE - len, "\n");

    write(STDOUT_FILENO, buffer, len);

    for (int i = 0; i < num_arrays; i++) {
        free(arrays[i]);
    }
    free(arrays);
    free(result);
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}
