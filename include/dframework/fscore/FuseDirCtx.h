#ifndef DFRAMEWORK_FSCORE_FUSEDIRCTX_H
#define DFRAMEWORK_FSCORE_FUSEDIRCTX_H

#include <dframework/fscore/FuseOpenCtx.h>
#include <dframework/lang/String.h>
#include <dframework/util/ArraySorted.h>

#ifndef _WIN32
#ifdef __cplusplus
namespace dframework {

    class FuseDirCtx : public FuseOpenCtx
    {
    private:
        bool     m_bFilled;
        char*    m_pContents;
        size_t   m_iLength;
        size_t   m_iSize;
        ArraySorted<String> m_nameList;

    public:
        FuseDirCtx();
        virtual ~FuseDirCtx();

        sp<Retval> extend(size_t minsize);
        sp<Retval> fill(const sp<FuseRequest>& req
                      , const char* in_name, const struct stat* in_st);

        inline void setFilled() { m_bFilled = true; }
        inline bool isFilled() { return m_bFilled; }
        inline void* getBuffer(unsigned offset){
            return ((void*)(m_pContents + offset));
        }
        inline size_t length() { return m_iLength; }
    };

}; // end namespace dframework
#endif // end cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FUSEDIRCTX_H */

