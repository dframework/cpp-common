#ifndef DFRAMEWORK_FILEDB_FILEDBSORTEDARRAY_H
#define DFRAMEWORK_FILEDB_FILEDBSORTEDARRAY_H
#define DFRAMEWORK_FILEDB_FILEDBSORTEDARRAY_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/io/DirBox.h>
#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {
namespace filedb {

    class FiledbNamedDir;
    class FiledbLastedDir;
    class FiledbSortedArray;

    class FiledbDirBase : public Object
    {
    private:
        sp<DirBox>  m_pDirbox;
        String      m_sName;
        dfw_time_t  m_uLastAcces;

    public:
        FiledbDirBase(const String& name);
        FiledbDirBase(const sp<DirBox>& dirbox);
        virtual ~FiledbDirBase();

        friend class FiledbNamedDir;
        friend class FiledbSortedArray;
        friend class FiledbLastedDir;
    };

    // --------------------------------------------------------------

    class FiledbNamedDir : public Object
    {
    private:
        sp<FiledbDirBase> m_pSource;

    public:
        FiledbNamedDir(const sp<FiledbDirBase>& source);
        inline virtual ~FiledbNamedDir() {}

        DFW_OPERATOR_EX_DECLARATION(FiledbNamedDir, m_pSource->m_sName);

        friend class FiledbSortedArray;
    };

    // --------------------------------------------------------------

    class FiledbLastedDir : public Object
    {
    private:
        sp<FiledbDirBase> m_pSource;

    public:
        FiledbLastedDir(const sp<FiledbDirBase>& source);
        inline virtual ~FiledbLastedDir() {}

        DFW_OPERATOR_EX_DECLARATION(FiledbLastedDir, m_pSource->m_uLastAcces);

        friend class FiledbSortedArray;
    };

    // --------------------------------------------------------------

    class FiledbSortedArray : public Object
    {
    private:
        ArraySorted<FiledbNamedDir>  m_aNamedList;

    public:
        FiledbSortedArray();
        virtual ~FiledbSortedArray();

        sp<DirBox> get(const char* uri) const;
        sp<DirBox> get(const URI& uri) const;
        sp<DirBox> get(const String& uri) const;
        sp<DirBox> get(int position) const;

        sp<Retval> insert(const sp<DirBox>& dirbox);

        inline int size() const { return m_aNamedList.size(); }

    };

}; // end namespace filedb
}; // end namespace dframework
#endif // end if cplusplus

#endif /* DFRAMEWORK_FILEDB_FILEDBSORTEDARRAY_H */

