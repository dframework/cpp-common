#include <dframework/io/DirBox.h>
#include <dframework/io/Dir.h>

namespace dframework {

    DirBox::DirBox(){
    }

    DirBox::DirBox(const char* path)
            : Object()
    {
        load(path);
    }

    DirBox::DirBox(const String& path)
            : Object()
    {
        load(path);
    }

    DirBox::DirBox(const URI& path)
            : Object()
    {
        load(path);
    }

    DirBox::~DirBox(){
    }

    sp<Retval> DirBox::load(const char* path){
        m_Uri = path;
        return (m_LastRetval = load());
    }

    sp<Retval> DirBox::load(const String& path){
        m_Uri = path;
        return (m_LastRetval = load());
    }

    sp<Retval> DirBox::load(const URI& path){
        m_Uri = path;
        return (m_LastRetval = load());
    }

    sp<Retval> DirBox::load(){
        sp<Retval> retval;
        Dir dir;
        String name;

        retval = dir.open(m_Uri);
        if(DFW_RETVAL_H(retval)) return DFW_RETVAL_D(retval);

        while( true ){
            // retval : DFW_T_COMPLETE or error code
            retval = dir.read(name);
            if(DFW_RETVAL_H(retval)) break;
            if( name.equals(".") || name.equals("..") ) continue;

            String sFilepath = String::format("%s/%s"
                , m_Uri.getPath().toChars()
                , name.toChars()
            );

            sp<Stat> st = new Stat();
            retval = st->stat(sFilepath);
            if(DFW_RETVAL_H(retval)) break;

            st->setName(name);
            m_aList.insert(st);
        }

        if( DFW_T_COMPLETE == DFW_RETVAL_V(retval) )
            return NULL;
        return retval;
    }

    String DirBox::dump(){
        String r = String::format(
                         "  @ DirBox:: %s"
                       , m_Uri.getPath().toChars()
                   );
        for(int k=0; k<size(); k++){
            sp<Stat> st = get(k);
            r.appendFmt(
                  "\n    `- %c: %s"
                , (st->isDir() ? 'D' : 'F')
                , st->getUri().getPath().toChars()
            );
        }
        return r;
    }

};

