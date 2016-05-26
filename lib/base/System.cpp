#include <dframework/base/System.h>
#include <dframework/io/File.h>
#include <dframework/net/URI.h>
#include <locale.h>

#if defined(__ANDROID__) || defined(__WIN32)
#include <dframework/android/langinfo.h>
#else
#include <langinfo.h>
#endif

namespace dframework {

    System::System(){
    }

    System::~System(){
    }

    DFW_STATIC
    const char* System::encoding(){
        setlocale(LC_ALL, "");
        setlocale(LC_CTYPE, NULL);
        return ::nl_langinfo(CODESET);
    }

#if !defined(_WIN32)
    DFW_STATIC
    sp<Retval> System::fork(int* out_pid){
        sp<Retval> retval;
        int eno = 0;
        dfw_retno_t rno = DFW_ERROR;
        const char* msg = NULL;

        int pid;
        unsigned count = 0;
        do{
            count++;
            if( -1 == (pid = ::fork()) ){
                eno = errno;
                if(eno == EAGAIN){
                    if(count > (10*5)){
                        return DFW_RETVAL_NEW_MSG(rno, eno
                                  , "errno=EAGAIN, count=%d, "
                                    "Don't fork."
                                  , count);
                    }
                    usleep(100*1000);
                    continue;
                }
                msg = Retval::errno_short(&rno, eno, "Not fork.");
                return DFW_RETVAL_NEW_MSG(rno, eno, "%s", msg);
            }
            break;
        }while(true);
        if( out_pid ) *out_pid = pid;
        return NULL;
    }
#endif

    DFW_STATIC
    sp<Retval> System::getpname(String& outname, int pid){
        sp<Retval> retval;
        String sContents;
        String sFname = String::format("/proc/%u/cmdline", pid);
        if( DFW_RET(retval, File::contents(sContents, sFname.toChars())) )
            return DFW_RETVAL_D(retval);

        URI::FileInfo fi;
        fi.parse(sContents.toChars());
        if(fi.m_sName.empty()){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Has not pid name. pid=%u", pid);
        }
        outname = fi.m_sName;
        return NULL;
    }

}; // end namespace dframework

