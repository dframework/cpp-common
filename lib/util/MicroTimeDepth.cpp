#include <dframework/util/MicroTimeDepth.h>

namespace dframework {

    MicroTimeDepth::MicroTimeDepth(){
    }

    MicroTimeDepth::~MicroTimeDepth(){
    }

    void MicroTimeDepth::clear(){
        m_Src.tv_sec = 0;
        m_Src.tv_usec = 0;
        m_Dst.tv_sec = 0;
        m_Dst.tv_usec = 0;
    }

    sp<Retval> MicroTimeDepth::start(){
        if( -1 == gettimeofday(&m_Src, NULL) ){
            int eno = errno;
            return DFW_RETVAL_NEW(DFW_ERROR,eno);
        }
        return NULL;
    }

    sp<Retval> MicroTimeDepth::stop(){
        struct timeval tv;
        if( -1 == gettimeofday(&tv, NULL) ){
            int eno = errno;
            return DFW_RETVAL_NEW(DFW_ERROR,eno);
        }
        m_Dst.tv_sec = tv.tv_sec - m_Src.tv_sec;
        m_Dst.tv_usec = tv.tv_usec - m_Src.tv_usec;
        if(m_Dst.tv_usec<0){
            --m_Dst.tv_sec;
              m_Dst.tv_usec += 1000000;
        }
        return NULL;
    }

    sp<Retval> MicroTimeDepth::stop(struct timeval *result){
        sp<Retval> retval = stop();
        if(DFW_RETVAL_H(retval))
            return DFW_RETVAL_D(retval);
        if(result){
            result->tv_sec = sec();
            result->tv_usec = usec();
        }
        return NULL;
    }

    long MicroTimeDepth::sec(){
        return m_Dst.tv_sec;
    }

    long MicroTimeDepth::usec(){
        return m_Dst.tv_usec;
    }

    String MicroTimeDepth::format(const char *fmt){
        return String::format(fmt, m_Dst.tv_sec, m_Dst.tv_usec);
    }

};

