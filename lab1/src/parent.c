#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 1024

void error_handler(const char *msg) {
    write(STDERR_FILENO, msg, strlen(msg));
    write(STDERR_FILENO, "\n", 1);
    exit(EXIT_FAILURE);
}

int main() {
    int pipe1[2];
    pid_t child_pid;
    char filename[BUFFER_SIZE];
    ssize_t bytesRead;

    const char *prompt = "Введите имя файла: ";
    write(STDOUT_FILENO, prompt, strlen(prompt));

    bytesRead = read(STDIN_FILENO, filename, sizeof(filename));
    if (bytesRead <= 0) {
        error_handler("Ошибка чтения имени файла");
    }

    if (filename[bytesRead - 1] == '\n') {
        filename[bytesRead - 1] = '\0';
    }

    printf("[INFO] Создаем pipe...\n");
    if (pipe(pipe1) == -1) {
        error_handler("Ошибка создания pipe");
    }

    printf("[INFO] Создаем дочерний процесс...\n");
    child_pid = fork();
    if (child_pid == -1) {
        error_handler("Ошибка создания процесса");
    }

    if (child_pid == 0) {
        close(pipe1[0]);

        printf("[INFO] Дочерний процесс: перенаправляем вывод в pipe...\n");
        if (dup2(pipe1[1], STDOUT_FILENO) == -1) {
            error_handler("Ошибка перенаправления вывода");
        }
        close(pipe1[1]);

        printf("[INFO] Дочерний процесс: выполняем child...\n");
        execl("./child", "child", filename, NULL);
        error_handler("Ошибка выполнения дочернего процесса");
    } else {
        close(pipe1[1]);

        char buffer[BUFFER_SIZE];
        ssize_t readBytes;
        printf("[INFO] Родительский процесс: читаем из pipe...\n");
        while ((readBytes = read(pipe1[0], buffer, BUFFER_SIZE)) > 0) {
            if (write(STDOUT_FILENO, buffer, readBytes) == -1) {
                error_handler("Ошибка записи в консоль");
            }
        }
        if (readBytes == -1) {
            error_handler("Ошибка чтения из pipe");
        }

        close(pipe1[0]);
        wait(NULL);
        printf("[INFO] Родительский процесс: завершено ожидание дочернего процесса.\n");
        exit(EXIT_SUCCESS);
    }

    return 0;
}
