#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// the name of the busybox binary in the initrd image
#define BUSYBOX_BINARY "/usr/bin/busybox"

#define _STR(x) #x
#define STR(x) _STR(x)

#define try(call)                                                                                                      \
    do {                                                                                                               \
        if ((call) != 0) {                                                                                             \
            error(STR(__LINE__) ": " #call " failed", errno);                                                          \
        }                                                                                                              \
    } while (false)

#ifdef DEBUG
#define log(msg, ...) printf(msg "\n", __VA_ARGS__)
#else
#define log(msg, ...)
#endif

volatile pid_t init_pid = 0;

typedef struct {
    const char* root_path;
    const char* init_path;

    int init_argc;

    char* const* init_argv;
    char* const* init_argp;
} args_t;

[[noreturn]] static void error(const char* msg, int err) {
    fprintf(stderr, "ERROR: %s: %s (%d)\n", msg, strerror(err < 0 ? -err : err), err);
    exit(EXIT_FAILURE);
}

static int parse_args(int argc, char* const argv[], char* const argp[], args_t* out) {
    int i;

    for (i = 1; i < argc; i++) {
        const auto arg = argv[i];

        // advance until first positional argument
        if (strlen(arg) > 0 && arg[0] != '-') {
            break;
        }

        // skip the -d flag's argument (capabilities list)
        if (strcmp(arg, "-d") == 0) {
            i += 1;
        }
    }

    if (i + 1 >= argc) {
        return EINVAL;
    }

    out->root_path = argv[i + 0];
    out->init_path = argv[i + 1];
    out->init_argc = argc - i - 2;
    out->init_argv = argv + i + 2;
    out->init_argp = argp;

    return 0;
}

static int init_entry(void* arg) {
    const auto args = (args_t*)arg;

    // parent already moved to the real root; /proc needs to be replaces
    try(umount("/proc"));
    try(mount("proc", "/proc", "proc", MS_NOEXEC | MS_NODEV | MS_NOSUID | MS_RELATIME, nullptr));

    char* argv[args->init_argc + 2];
    argv[0] = (char*)args->init_path;
    argv[args->init_argc + 1] = nullptr;

    // this is pretty unsave, but whatever
    memcpy(argv + 1, args->init_argv, sizeof(char*) * (uint64_t)args->init_argc);

    execve(args->init_path, (char* const*)argv, args->init_argp);
    error("execve to init binary should not return", errno);
}

static void forward_sigint(int sig) {
    if (init_pid != 0) {
        kill(init_pid, sig);
    }
}

int main(int argc, char* const argv[], char* const argp[]) {
    // dry runs (e.g. `run-init -n`) are invoked from a forked shell, so PID is not 1
    if (getpid() != 1) {
        execve(BUSYBOX_BINARY, argv, argp);
        error("execve to busybox run-init should not return", errno);
    }

    args_t args = {0};
    try(parse_args(argc, argv, argp, &args));

    // move to the real root in the parent's mount namespace before cloning
    try(chdir(args.root_path));
    try(mount(args.root_path, "/", nullptr, MS_MOVE, nullptr));
    try(chroot("."));
    try(chdir("/"));

    const auto stack_size = 1024UL * 1024UL;

    // allocate the stack memory
    auto stack_buffer = (char*)malloc(stack_size);
    if (stack_buffer == nullptr) {
        error("could not allocate stack", ENOMEM);
    }

    init_pid = clone(
        /* fn = */ init_entry,
        /* stack = */ stack_buffer + stack_size, // on x86 the stack grows downwards.
        /* flags = */ CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
        /* arg = */ &args,
        /* parent_tid = */ nullptr,
        /* tls = */ nullptr,
        /* child_tid = */ nullptr
    );
    if (init_pid < 0) {
        error("clone init process failed", errno);
    }

    log("child process started (pid: %d)", init_pid);

    struct sigaction sigact = {
        .sa_handler = forward_sigint,
        .sa_flags = 0
    };
    sigemptyset(&sigact.sa_mask);
    try(sigaction(SIGINT, &sigact, nullptr));

    int status = 0;
    while (true) {
        const auto ret = waitpid(init_pid, &status, 0);

        // systemd signals shutdown with 0 return code, otherwise reboot
        if (ret == -1 && errno == EINTR) {
            continue;
        }

        if (ret == -1) {
            error("waitpid failed", errno);
        }

        break;
    }

    if (WIFEXITED(status)) {
        log("init process exited (status: %d)", WEXITSTATUS(status));

        // systemd signals shutdown with 0 return code, otherwise reboot
        if (WEXITSTATUS(status) == 0) {
            reboot(RB_POWER_OFF);
        }
    }

    if (WIFSIGNALED(status)) {
        log("init process killed (signal: %d)", WTERMSIG(status));
    }

    reboot(RB_AUTOBOOT);
    error("end of program should be unreachable", -1);
}
