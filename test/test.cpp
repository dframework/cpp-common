#include <dframework/lang/String.h>
#include <dframework/net/URI.h>

using namespace dframework;

int main(){
    String hello = "hello world";
    printf("%s\n", hello.toChars());
    //URI uri = "http://home-1.imgtech.co.kr:8080/a/b/c.mp4?a=b#fra";
    //URI uri = "/a/b/c.mp4?a=b#fra";
    URI uri = "file:///a/b/c.mp4?a=b#fra";
    
	printf(">>> #2: %s\n", uri.getHost().toChars());
	printf(">>> #3: %s\n", uri.getPath().toChars());
	printf(">>> #4: %s\n", uri.getQuery().toChars());
	printf(">>> #5: %s\n", uri.getFragment().toChars());
    return 0;
}

