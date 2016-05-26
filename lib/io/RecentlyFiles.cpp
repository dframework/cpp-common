#include <dframework/io/RecentlyFiles.h>

namespace dframework {

    DFW_DECLARATION_SAFE_C_COUNT(RecentlyFiles::Time, Time);    

    RecentlyFiles::Time::Time(sp<String>& name, dfw_time_t time
                            , sp<Object>& ctx)
            : Object()
    {
        DFW_SAFE_C_ADD(RecentlyFiles::Time, Time, l);

        m_sName = name;
        m_iTime = time;
        m_context = ctx;
    }

    RecentlyFiles::Time::Time(sp<String>& name, dfw_time_t time)
            : Object()
    {
        DFW_SAFE_C_ADD(RecentlyFiles::Time, Time, l);

        m_sName = name;
        m_iTime = time;
    }

    RecentlyFiles::Time::~Time(){
        DFW_SAFE_C_REMOVE(RecentlyFiles::Time, Time, l);
    }

    // --------------------------------------------------------------

    DFW_DECLARATION_SAFE_C_COUNT(RecentlyFiles::Name, Name);

    RecentlyFiles::Name::Name(sp<String>& name, sp<RecentlyFiles::Time>& time)
            : Object()
    {
        DFW_SAFE_C_ADD(RecentlyFiles::Name, Name, l);
        m_sName = name;
        m_oTime = time;
    }

    RecentlyFiles::Name::Name(sp<String>& name)
            : Object()
    {
        DFW_SAFE_C_ADD(RecentlyFiles::Name, Name, l);
        m_sName = name;
    }

    RecentlyFiles::Name::~Name(){
        DFW_SAFE_C_REMOVE(RecentlyFiles::Name, Name, l);
    }

    // --------------------------------------------------------------

    DFW_DECLARATION_SAFE_COUNT(RecentlyFiles);

    RecentlyFiles::RecentlyFiles(){
        DFW_SAFE_ADD(RecentlyFiles, l);
    }

    RecentlyFiles::~RecentlyFiles(){
        DFW_SAFE_REMOVE(RecentlyFiles, l);
    }

    sp<RecentlyFiles::Time> RecentlyFiles::getTimeNode(int position){
        AutoLock _l(this);
        return m_aTimes.get(position);
    }

    int RecentlyFiles::size(){
        AutoLock _l(this);
        return m_aNames.size();
    }

    bool RecentlyFiles::hasRecently(const char* name){
        AutoLock _l(this);
        sp<String> sName = new String(name);
        sp<Name> oName = new Name(sName);
        if( m_aNames.index(oName) == -1 )
            return false;
        return true;
    }

    sp<Retval> RecentlyFiles::add(const char* name, dfw_time_t time
                                , sp<Object>& ctx){
        AutoLock _l(this);
        sp<Retval> retval;

        sp<String> sName = new String(name);
        sp<Name> oName = new Name(sName);
        sp<Name> sSearch = m_aNames.get(oName);
        if( !sSearch.has() ){
            sp<Time> oTime = new Time(sName, time, ctx);
            oName->m_oTime = oTime;
            if( DFW_RET(retval, m_aNames.insert(oName)) )
                return DFW_RETVAL_D(retval);
            if( DFW_RET(retval, m_aTimes.insert(oTime)) ){
                m_aNames.remove(sName);
                return DFW_RETVAL_D(retval);
            }
            return NULL;
        }

        if( DFW_RET(retval, updateTime(sName
                                     , sSearch->m_oTime->m_iTime
                                     , time))){
            return DFW_RETVAL_D(retval);
        }

        return NULL;
    }

    sp<Retval> RecentlyFiles::remove(const char* name){
        AutoLock _l(this);
        sp<Retval> retval;

        sp<String> sName = new String(name);
        sp<Name> oName = new Name(sName);
        sp<Name> sSearch = m_aNames.get(oName);
        if( sSearch.has() ){
            m_aNames.remove(sSearch);
            if( DFW_RET(retval, removeTime(sSearch->m_oTime)) )
                return DFW_RETVAL_D(retval);
        }
        return NULL;
    }

    sp<Retval> RecentlyFiles::removeTime(sp<Time>& fTime)
    {
        AutoLock _l(this);
        sp<Retval> retval;

        int index;
        if( -1 == (index = m_aTimes.index(fTime)) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Has not file time node. path=%s, time=%d"
                       , fTime->m_sName->toChars()
                       , fTime->m_iTime);
        }

        for(int k=index;;k--){
            sp<Time> temp = m_aTimes.get(k);
            if( temp.has() 
                    && (temp->m_iTime == fTime->m_iTime)
                    && temp->m_sName->equals(fTime->m_sName->toChars() )){
                m_aTimes.remove(k);
                return NULL;
            }else if( !temp.has() ){
                break;
            }
        }

        for(int k=index+1;;k++){
            sp<Time> temp = m_aTimes.get(k);
            if( temp.has() 
                    && (temp->m_iTime == fTime->m_iTime)
                    && temp->m_sName->equals(fTime->m_sName->toChars() )){
                m_aTimes.remove(k);
                return NULL;
            }else if( !temp.has() ){
                break;
            }
        }

        return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not found time node. path=%s, time=%d"
                   , fTime->m_sName->toChars()
                   , fTime->m_iTime);
    }

    sp<Retval> RecentlyFiles::updateTime(sp<String>& name
                                       , dfw_time_t old, dfw_time_t val)
    {
        AutoLock _l(this);
        sp<Retval> retval;

        int index;
        sp<Time> fTime = new Time(name, old);
        if( -1 == (index = m_aTimes.index(fTime)) ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                       , "Has not file time node. path=%s, old=%d, new=%d"
                       , name->toChars(), old, val);
        }

        for(int k=index;;k--){
            sp<Time> temp = m_aTimes.get(k);
            if( temp.has() 
                    && (temp->m_iTime == old)
                    && temp->m_sName->equals(name->toChars() )){
                m_aTimes.remove(k);
                temp->m_iTime = val;
                if( DFW_RET(retval, m_aTimes.insert(temp)) )
                    return DFW_RETVAL_D(retval);
                return NULL;
            }else if( !temp.has() ){
                break;
            }
        }

        for(int k=index+1;;k++){
            sp<Time> temp = m_aTimes.get(k);
            if( temp.has() 
                    && (temp->m_iTime == old)
                    && temp->m_sName->equals(name->toChars() )){
                m_aTimes.remove(k);
                temp->m_iTime = val;
                if( DFW_RET(retval, m_aTimes.insert(temp)) )
                    return DFW_RETVAL_D(retval);
                return NULL;
            }else if( !temp.has() ){
                break;
            }
        }

        return NULL;
    }


};

