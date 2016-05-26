#include <dframework/base/Thread.h>
#include <signal.h>

using namespace dframework;

class A : public Thread
{
public:
	A() { printf("create a\n");}
 	virtual ~A() { printf("delete a\n");}

	virtual void run(){
#if 0
		int sleep_cnt = 0;
		setSigno(SIGUSR1);
		while(1){
			printf("thread ... %p\n", this);
			sleep(1);
			sleep_cnt++;
			//if(sleep_cnt>3) break;
        	}
#endif
	}
	virtual void cleanup(){
		printf("cleanup ... \n");
	}
        virtual void onSignal(int signo){
		printf("on signal : %d\n", signo);
		sleep(2);
		printf("on signal after : %d\n", signo);
	}
};

int main(void)
{

	{
		printf("###### start\n");
		sp<A> a = new A();
		printf("#start\n");
		a->start();
#if 0
		printf("#join\n");
		a->join();
		printf("#join after\n");
#else
		for(int k=0; k<6; k++){
			if(k==3){
				printf("kill\n");
				a->kill(SIGUSR1);
				printf("kill after\n");
			}
			sleep(1);
			printf("main...\n");
		}
#endif

	}

		printf("##### end\n");
	return 0;

}

