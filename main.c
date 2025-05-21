#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <poll.h>

#define TIMEOUT 10000 // 10 seconds

volatile int ask_mode = 1; // 1 for ask mode, 0 for no ask mode

void ask_user_action() {
    if (!ask_mode) return;

    printf("\nФункція зависає. Оберіть дію:\n");
    printf("1. Продовжити виконання\n");
    printf("2. Завершити виконання\n");
    printf("3. Продовжити, не питати більше\n");
    fflush(stdout);

    int choice;
    if (scanf("%d", &choice) != 1) choice = 2;
    if (choice == 2) exit(0);
    if (choice == 3) ask_mode = 0;
}

int run_function(const char* exec_name, int x, int* result) {
    int pipe_in[2], pipe_out[2];
    pipe(pipe_in); // [1] - write to child
    pipe(pipe_out); // [0] - read from child

    pid_t pid = fork();
    if (pid == 0) {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_in[1]);
        close(pipe_out[0]);
        close(pipe_out[1]);
        execlp(exec_name, exec_name, NULL);
        perror("execlp failed");
        exit(1);
    }

    close(pipe_in[0]);
    close(pipe_out[1]);

    dprintf(pipe_in[1], "%d\n", x);
    close(pipe_in[1]);

    struct pollfd pfd = { .fd = pipe_out[0], .events = POLLIN };
    char buffer[64];
    int total_wait = 0;

    while (1) {
        int ret = poll(&pfd, 1, TIMEOUT);
        if (ret >0) {
            if (fgets(buffer, sizeof(buffer), fdopen(pipe_out[0], "r")) != NULL) {
                *result = atoi(buffer);
                close(pipe_out[0]);
                waitpid(pid, NULL, 0);
                return 1;
            } else {
                break;
            }
        } else {
            ask_user_action();
        }
    }

    close(pipe_out[0]);
    waitpid(pid, NULL, 0);
    return 0; // No result received
}

int main(int argc, char* argv[]) {
    int x;
    printf("Input x: ");
    scanf("%d", &x);

    int f_val, g_val;
    int f_done = run_function("./f", x, &f_val);
    if (f_done && f_val == 0) {
        printf("Result: 0 (f == 0 => f && g == 0)\n");
        return 0;
    }

    int g_done = run_function("./g", x, &g_val);
    if (g_done && g_val == 0) {
        printf("Result: 0 (g == 0 => f && g == 0)\n");
        return 0;
    }

    if (f_done && g_done) {
        printf("Result: %d\n", f_val && g_val);
    } else {
        printf("Result: undefined (f or g did not finish)\n");
    }

    return 0;
}