#ifndef DFRAMEWORK_BASE_SIGACTION_H
#define DFRAMEWORK_BASE_SIGACTION_H


#if defined(_WIN32) || defined(_WIN64)
struct sigaction {
    void (*sa_handler)(int);
    int sa_mask;
    int sa_flags;
};


#define sigemptyset(x) memset((x), 0, sizeof(*(x)))


#ifdef __cplusplus
extern "C" {
#endif

int sigaction(int sig, struct sigaction *action, struct sigaction *old);

#ifdef __cplusplus
};
#endif

#endif

#endif /* DFRAMEWORK_BASE_SIGACTION_H */

