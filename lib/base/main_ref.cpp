#define DEBUG_TERMINAL 1
#include <dframework/base/type.h>
#include <dframework/base/sp.h>
#include <dframework/base/Retval.h>



using namespace dframework;

#if 0
static int hello_create_cnt = 0;
static int hello_delete_cnt = 0;

class Hello : public Ref
{
public:
    Hello();
    virtual ~Hello();
    int value;
};

Hello::Hello(){
    hello_create_cnt++;
    value = 10;
}

Hello::~Hello(){
    hello_delete_cnt++;
}

sp<Hello> get(){
    return NULL;
}
#endif

dfw_retval_t getRetvalLocal(){
    return {DFW_OK,0};
}

int main(void){

#if 0
    for(int k=0; k<1000; k++)
    {
        printf("----------- k=%d\n", k);
        sp<Hello> h1(new Hello());
    }
#endif

    dfw_retval_t k = getRetvalLocal();
    printf("retno=%d, errno=%d\n", k.m_retno, k.m_errno);

    return 0;
}

