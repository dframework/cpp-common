#include <dframework/http/HttpGet.h>
#include <dframework/http/HttpPropfind.h>


using namespace dframework;

static int cnt = 0;

class AAA : public Object
{
public:
    class HttpListener : public HttpQuery::OnHttpListener
    {
    public :
        virtual sp<Retval> onResponse(sp<HttpConnection>& c
                               , const char* b, dfw_size_t s){
            printf("size = %ld\n", s);
            return NULL;
        }
    };
    class PropfindListener : public HttpPropfind::OnPropfindListener
    {
    public :
        virtual sp<Retval> onPropfind(sp<HttpConnection>& c
                               , sp<HttpPropfind::Prop>& prop){
            printf("name => %s\r\n", prop->m_sName.toChars());
            return NULL;
        }
    };

public:
    sp<HttpQuery::OnHttpListener> m_HttpListener;
    sp<HttpPropfind::OnPropfindListener> m_PropfindListener;

	AAA() { 
		printf("create aaa\n"); 
		m_HttpListener = new HttpListener();
		m_PropfindListener = new PropfindListener();
	}
	virtual 	~AAA() { printf("delete aaa\n"); }

};

int main(void)
{

    sp<AAA> aaa = new AAA();
    sp<HttpPropfind> p = new HttpPropfind();
    p->setOnHttpListener(aaa->m_HttpListener);
    p->setOnPropfindListener(aaa->m_PropfindListener);
    sp<Retval> ret = p->query("http://drm.client.imgtech.co.kr/webdav/");

if(ret.has()){
printf("%s\n", ret->dump().toChars());
}else{
printf("OK\n");
}

    return 0;
}

