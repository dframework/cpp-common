#include <dframework/base/Sigaction.h>
#include <signal.h>

#if defined(_WIN32) || defined(_WIN64)

int sigaction(int sig, struct sigaction *action, struct sigaction *old){
    if(sig == -1)
        return 0;
    if( !old ){
        if (signal(sig, SIG_DFL) == SIG_ERR)
            return -1;
    }else{
        if (signal(sig, action->sa_handler) == SIG_ERR)
            return -1;
    }
    return 0;
}

#endif

