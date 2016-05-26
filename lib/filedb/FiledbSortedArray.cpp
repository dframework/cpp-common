#include <dframework/filedb/FiledbSortedArray.h>
#include <dframework/util/Time.h>

namespace dframework {
namespace filedb {

    FiledbDirBase::FiledbDirBase(const String& name)
            : Object()
    {
        m_sName = name;
    }

    FiledbDirBase::FiledbDirBase(const sp<DirBox>& dirbox)
            : Object()
    {
        m_pDirbox = dirbox;
        m_sName = m_pDirbox->getUri().toString();
    }

    FiledbDirBase::~FiledbDirBase(){
    }

    // --------------------------------------------------------------

    FiledbNamedDir::FiledbNamedDir(const sp<FiledbDirBase>& source)
            : Object()
    {
        m_pSource = source;
    }

    // --------------------------------------------------------------

    FiledbLastedDir::FiledbLastedDir(const sp<FiledbDirBase>& source)
            : Object()
    {
        m_pSource = source;
    }

    // --------------------------------------------------------------

    FiledbSortedArray::FiledbSortedArray(){
    }

    FiledbSortedArray::~FiledbSortedArray(){
    }

    sp<DirBox> FiledbSortedArray::get(const char* uri) const{
        URI oUri = uri;
        return get(oUri);
    }
    
    sp<DirBox> FiledbSortedArray::get(const String& uri) const{
        URI oUri = uri;
        return get(oUri);
    }

    sp<DirBox> FiledbSortedArray::get(const URI& uri) const{
        String sName = uri.toString();
        sp<FiledbDirBase> base = new FiledbDirBase(sName);
        sp<FiledbNamedDir> named  = new FiledbNamedDir(base);
        sp<FiledbNamedDir> find = m_aNamedList.get(named);
        if(find.has() && find->m_pSource.has())
            return find->m_pSource->m_pDirbox;
        return NULL;
    }

    sp<DirBox> FiledbSortedArray::get(int position) const{
        sp<FiledbNamedDir> find = m_aNamedList.get(position);
        if(find.has() && find->m_pSource.has())
            return find->m_pSource->m_pDirbox;
        return NULL;
    }

    sp<Retval> FiledbSortedArray::insert(const sp<DirBox>& dirbox) {
        if( !dirbox.has() )
            return DFW_RETVAL_NEW(DFW_E_INVAL, 0);

        sp<FiledbDirBase>   base   = new FiledbDirBase(dirbox);
        sp<FiledbNamedDir>  named  = new FiledbNamedDir(base);

        return m_aNamedList.insert(named);
    }

}; // end namespace filedb
}; // end namespace dframework

