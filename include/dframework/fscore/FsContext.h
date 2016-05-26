#ifndef DFRAMEWORK_FSCORE_FSCONTEXT_H
#define DFRAMEWORK_FSCORE_FSCONTEXT_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/util/NamedObject.h>

#ifdef __cplusplus
#ifndef _WIN32
namespace dframework {

    class FsContext : public Object
    {
    public:
        DFW_DEFINITION_SAFE_COUNT(FsContext);

    private:
        String m_mountpoint;

    public:
        ArraySorted<NamedObject> m_aList;

    public:
        FsContext();
        virtual ~FsContext();

        void setMountpoint(const char* mp) { m_mountpoint = mp; }
        const char* getMountpoint() { return m_mountpoint.toChars(); }

        int contextSize();

        sp<Retval> insertContext(sp<NamedObject>& ctx);

        sp<NamedObject> removeContext(sp<NamedObject>& ctx);

        sp<NamedObject> getContext(sp<NamedObject>& ctx);

        sp<NamedObject> getContext(int position);

        sp<Object> getContext(const char* named);

        inline sp<Object> getContext(const String& named){
            return getContext(named.toChars());
        }

        void clear();

    };

}; // end namespae dframework
#endif // end if cplusplus
#endif

#endif /* DFRAMEWORK_FSCORE_FSCONTEXT_H */

