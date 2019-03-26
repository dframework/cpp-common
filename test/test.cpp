#include <dframework/lang/String.h>
#include <dframework/net/URI.h>

using namespace dframework;

int main(){
    String hello = "hello world";
    printf("%s\n", hello.toChars());
    URI uri = "http://home-1.imgtech.co.kr:8080";
    //URI uri;
//	uri.parse("http://home-1.imgtech.co.kr/a/b/c.mp4");
    
	printf(">>> #2: %s\n", uri.getHost().toChars());
    return 0;
}

