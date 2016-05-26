#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>


using namespace dframework;

class A : public Object
{
};

int main(void)
{
	printf("start\n");
	//ArraySorted<A> alist;
	for(int k=0; k>=0 ; k++){
		sp<A> a = new A();
		/*sp<Retval> ret = alist.insert(a);
		if(ret.has())
			printf("ret error %s\n", ret->dump().toChars());
		else
			printf("%d .. ok ..\n", k);
		*/
		AutoLock _l(a.get());
			printf("%d\n", k);
	}
	printf("end\n");
}

