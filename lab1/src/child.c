#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>

#define BUFFER_SIZE 1024

void HandleError(const char *message) {
    write(STDERR_FILENO, message, strlen(message));
    exit(EXIT_FAILURE);
}

int safe_strtol(const char *str, char **endptr) {
    long value = strtol(str, endptr, 10);

    if (value > INT_MAX || value < INT_MIN) {
        HandleError("Ошибка: значение выходит за пределы допустимого диапазона int.\n");
    }
    return (int)value;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        HandleError("Ошибка: необходимо указать путь к файлу.\n");
    }

    int file = open(argv[1], O_RDONLY);
    if (file == -1) {
        char error_msg[BUFFER_SIZE];
        snprintf(error_msg, sizeof(error_msg), "Ошибка открытия файла '%s': %s\n", argv[1], strerror(errno));
        HandleError(error_msg);
    }

    int output_fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fd == -1) {
        HandleError("Ошибка создания файла output.txt.\n");
    }

    if (dup2(file, STDIN_FILENO) == -1) {
        HandleError("Ошибка перенаправления ввода.\n");
    }
    close(file);

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    char *current;
    char output[BUFFER_SIZE];
    int output_len;

    while ((bytesRead = read(STDIN_FILENO, buffer, BUFFER_SIZE)) > 0) {
        current = buffer;

        while (current < buffer + bytesRead) {
            while (*current == ' ' || *current == '\t') {
                current++;
            }
            if (*current == '\n') {
                current++;
                continue;
            }

            char *endptr;
            int first_number = safe_strtol(current, &endptr);
            int original_number = first_number;
            current = endptr;

            output_len = snprintf(output, BUFFER_SIZE, "Число: %d", original_number);

            while (1) {
                while (*current == ' ' || *current == '\t') {
                    current++;
                }

                if (*current == '\n' || current >= buffer + bytesRead) {
                    break;
                }

                int next_number = safe_strtol(current, &endptr);
                if (next_number == 0) {
                    output_len = snprintf(output, BUFFER_SIZE, "Ошибка: деление на ноль.\n");
                    write(STDOUT_FILENO, output, output_len);
                    close(output_fd);
                    exit(EXIT_FAILURE);
                }

                output_len += snprintf(output + output_len, BUFFER_SIZE - output_len, ", %d / %d = %d", original_number, next_number, original_number / next_number);

                current = endptr;
            }

            output[output_len++] = '\n';

            write(STDOUT_FILENO, output, output_len);
            write(output_fd, output, output_len);

            while (*current != '\n' && current < buffer + bytesRead) {
                current++;
            }
            if (*current == '\n') {
                current++;
            }
        }
    }

    if (bytesRead == -1) {
        HandleError("Ошибка чтения файла.\n");
    }

    close(output_fd);
    exit(EXIT_SUCCESS);
}
