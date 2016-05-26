#include <dframework/base/type.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySelfSorted.h>
#include <dframework/util/ArrayLinked.h>
#include <dframework/io/Dir.h>
#include <dframework/io/DirBox.h>
#include <time.h>
#include <dframework/filedb/FiledbSortedArray.h>
#include <dframework/net/Hostname.h>
#include <dframework/lang/Integer.h>

using namespace dframework;
using namespace dframework::filedb;

static int count = 0;
static int dount = 0;

class Test : public Object
{
public:
    int value;

    inline Test(int in) {
        value = in;
        count++;
        //printf("create Test <%d>\n", value);
    }
    inline virtual ~Test() { 
        dount++;
        //printf("drop Test <%d>\n", value);
    }

    DFW_OPERATOR_EX_DECLARATION(Test, value);

};



int main(){

	ArraySorted<Integer> il;
	ArraySorted<String> sl;

	il.insert( new Integer(100) );
	il.insert( new Integer(80) );
	il.insert( new Integer(120) );
	il.insert( new Integer(70) );
	il.insert( new Integer(110) );
	il.insert( new Integer(10) );
	il.insert( new Integer(50) );
	il.insert( new Integer(20) );
	il.insert( new Integer(60) );
	il.insert( new Integer(130) );
       
	sl.insert( new String("k") );
#if 1
	sl.insert( new String("s") );
	sl.insert( new String("c") );
	sl.insert( new String("d") );
	sl.insert( new String("a") );
	sl.insert( new String("z") );
	sl.insert( new String("y") );
#endif

	printf("isize : %d\n", il.size());
	for(int k=0; k<il.size(); k++){
                
		printf("  -- %d => %d\n", k, il.get(k)->value());
	}
	printf("ssize : %d\n", sl.size());
	for(int k=0; k<sl.size(); k++){
                
		printf("  -- %d => %s\n", k, sl.get(k)->toChars());
	}

	sp<String> f = new String("z");
	sp<String> res = sl.get(f);
	if( res.has() )
		printf("res : %s\n", res->toChars());
	else
		printf("not find\n");

	sp<Integer> fi = new Integer(130);
	sp<Integer> resi = il.get(fi);
	if( resi.has() )
		printf("resi : %d\n", resi->value());
	else
		printf("resi not find\n");

#if 0
    DirBox box;
    box.load("/opt");
    String boxdump = box.dump();
    printf("%s\n", boxdump.toChars());
    printf("dirbase: %s\n", box.getUri().toString().toChars());
#endif

#if 0
    String name;
    Dir dir("/opt");
    sp<Retval> test = dir.lastRetval();
    if( test.has() ){
        printf("error=>\n%s\n", test->dump().toChars());
        return 1;
    }
    
    while(true){
        sp<Retval> retval = dir.read(name);
        if(retval.has() && retval->value()==DFW_T_COMPLETE)
            break;
        printf("name: %s\n", name.toChars());
    }
#endif

#if 0
    printf("test\n");
    {

        srand(time(NULL));
	ArraySelfSorted<Test> list;
	for(int k=0; k<10; k++){
            int v= rand() % 10 +1;
        //printf("rand = %d\n", v);
    	    sp<Test> test = new Test(v);
    	    list.insert(test);
	}

	for(int k=0; k<list.size(); k++){
		sp<Test> t1 = list.get(k);
                printf("===> <%d>, addr=%p\n", t1->value, t1.get());
        }

	printf("out\n");
    }

    printf("end, count=%d, dount=%d\n", count, dount);
#endif

#if 0
	Hostname host;
	sp<Retval> retval = host.get("http://www.daum.net");
if(retval.has()){
String s = retval->dump();
printf("ERROR => %s\n", s.toChars());
}
        for(int k=0; k<host.size(); k++){
            String ele = host.ip(k);
		printf("ele: %s\n", ele.toChars());
//            sp<Hostname::Result> ele = host.get(k);
//		printf("ele: %s\n", ele->m_sIp.toChars());
        }
#endif

#if 0
	URI uri;
	sp<Retval> retval = uri.parse("http://www.daum.net");
        if(retval.has())
printf("%s\n", retval->dump().toChars());
else
printf("uri string: %s\n", uri.toString().toChars());
#endif

    return 0;
}

