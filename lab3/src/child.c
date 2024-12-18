#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

#define SHM_NAME "/shared_memory"
#define SEM_NAME "/sync_semaphore"
#define BUFFER_SIZE 1024
#define NUM_LINES 100

void HandleError(const char *message) {
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
}

int safe_strtol(const char *str, char **endptr) {
    long value = strtol(str, endptr, 10);
    if (value > INT_MAX || value < INT_MIN) {
        HandleError("Ошибка: значение выходит за пределы диапазона int.\n");
    }
    return (int)value;
}

int main() {
    int shm_fd;
    char *shared_mem;
    sem_t *semaphore;

    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        HandleError("Ошибка подключения к разделяемой памяти.\n");
    }

    shared_mem = mmap(NULL, BUFFER_SIZE * NUM_LINES, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        HandleError("Ошибка отображения разделяемой памяти.\n");
    }

    semaphore = sem_open(SEM_NAME, 0);
    if (semaphore == SEM_FAILED) {
        HandleError("Ошибка подключения к семафору.\n");
    }

    sem_wait(semaphore);

    char filename[BUFFER_SIZE];
    strncpy(filename, shared_mem, BUFFER_SIZE);

    int file = open(filename, O_RDONLY);
    if (file == -1) {
        strncpy(shared_mem, "Ошибка: Не удалось открыть файл.\n", BUFFER_SIZE);
        sem_post(semaphore);
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int first_number, next_number;
    char *current;
    int line_number = 0;

    while ((bytesRead = read(file, buffer, BUFFER_SIZE)) > 0) {
        current = buffer;

        while (current < buffer + bytesRead) {
            while (*current == ' ' || *current == '\t') current++;
            if (*current == '\n') {
                current++;
                continue;
            }

            char *endptr;
            first_number = safe_strtol(current, &endptr);
            current = endptr;

            char result[BUFFER_SIZE];
            int result_len = snprintf(result, BUFFER_SIZE, "Результат: %d", first_number);

            while (current < buffer + bytesRead && *current != '\n') {
                while (*current == ' ' || *current == '\t') current++;
                if (*current == '\n') break;

                next_number = safe_strtol(current, &endptr);
                if (next_number == 0) {
                    strncpy(shared_mem + line_number * BUFFER_SIZE, "Ошибка: Деление на ноль.\n", BUFFER_SIZE);
                    sem_post(semaphore);
                    exit(EXIT_FAILURE);
                }

                result_len += snprintf(result + result_len, BUFFER_SIZE - result_len, ", %d / %d = %d",
                                       first_number, next_number, first_number / next_number);
                current = endptr;
            }

            result[result_len++] = '\n';
            strncpy(shared_mem + line_number * BUFFER_SIZE, result, result_len);
            line_number++;

            if (line_number >= NUM_LINES) break;
        }
    }

    if (bytesRead == -1) {
        strncpy(shared_mem, "Ошибка чтения файла.\n", BUFFER_SIZE);
        sem_post(semaphore);
        exit(EXIT_FAILURE);
    }

    close(file);
    sem_post(semaphore);
    exit(EXIT_SUCCESS);
}
