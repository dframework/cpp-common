#include <dframework/lang/String.h>

using namespace dframework;

int main(){
    String hello = "hello world";
	printf("%s\n", hello.toChars());
	String re = hello.replace("o", "-reo-");
    return 0;
}

