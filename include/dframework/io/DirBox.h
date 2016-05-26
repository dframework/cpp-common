#ifndef DFRAMEWORK_IO_DIRBOX_H
#define DFRAMEWORK_IO_DIRBOX_H
#define DFRAMEWORK_IO_DIRBOX_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/net/URI.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/io/Stat.h>

#ifdef __cplusplus
namespace dframework {

    class DirBox : public Object 
    {
    private:
        sp<Retval> m_LastRetval;
        ArraySorted<Stat> m_aList;
        URI m_Uri;

    public:
        DirBox();
        DirBox(const char* path);
        DirBox(const String& path);
        DirBox(const URI& path);
        virtual ~DirBox();

        sp<Retval> load(const char* path);
        sp<Retval> load(const String& path);
        sp<Retval> load(const URI& path);

        String dump();

        inline const URI& getUri() const { return m_Uri; }
        inline int size() const { return m_aList.size(); }
        inline void clear() { m_aList.clear(); }
        inline sp<Stat> get(int index) { return m_aList.get(index); }
        inline sp<Retval> lastRetval() const { return m_LastRetval; }
        inline sp<Retval> insert(sp<Stat>& st) {
            return m_aList.insert(st); 
        }

        DFW_OPERATOR_EX_DECLARATION(DirBox, m_Uri);

    private:
        sp<Retval> load();
    
    };

};
#endif

#endif /* DFRAMEWORK_IO_DIRBOX_H */

