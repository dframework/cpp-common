#include <dframework/sql/Sqlite3.h>
#ifdef __APPLE__
#include "sqlite3/sqlite3.h"
#else
#include <sqlite3/sqlite3.h>
#endif

namespace dframework {

    Sqlite3Row::Sqlite3Row(unsigned no) : dframework::Object::Object()
    {
        m_no = no;
    }

    Sqlite3Row::~Sqlite3Row(){
    }

    int Sqlite3Row::size() {
        AutoLock _l(this);
        return m_aFields.size();
    }

    sp<NamedValue> Sqlite3Row::getField(int k) {
        AutoLock _l(this);
        return m_aFields.get(k);
    }

    int Sqlite3Row::getFieldInt(int k) {
        AutoLock _l(this);
        sp<NamedValue> nv = m_aFields.get(k);
        if(nv.has())
            return nv->getInt();
        return -1;
    }

    long Sqlite3Row::getFieldLong(int k) {
        AutoLock _l(this);
        sp<NamedValue> nv = m_aFields.get(k);
        if(nv.has())
            return nv->getLong();
        return -1;
    }

    String Sqlite3Row::getFieldString(int k) {
        AutoLock _l(this);
        sp<NamedValue> nv = m_aFields.get(k);
        if(nv.has())
            return nv->m_sValue;
        return NULL;
    }

    const char* Sqlite3Row::getFieldChars(int k) {
        AutoLock _l(this);
        sp<NamedValue> nv = m_aFields.get(k);
        if(nv.has())
            return nv->m_sValue.toChars();
        return NULL;
    }

    sp<NamedValue> Sqlite3Row::getField(String& name) {
        AutoLock _l(this);
        sp<NamedValue> find = new NamedValue(name);
        return m_aFields.get(find);
    }

    int Sqlite3Row::getFieldInt(String& name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->getInt();
        return -1;
    }

    long Sqlite3Row::getFieldLong(String& name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->getLong();
        return -1;
    }

    String Sqlite3Row::getFieldString(String& name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->m_sValue;
        return NULL;
    }

    const char* Sqlite3Row::getFieldChars(String& name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->m_sValue.toChars();
        return NULL;
    }

    sp<NamedValue> Sqlite3Row::getField(const char* name){
        AutoLock _l(this);
        sp<NamedValue> find = new NamedValue(name);
        return m_aFields.get(find);
    }

    int Sqlite3Row::getFieldInt(const char* name){
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->getInt();
        return -1;
    }

    long Sqlite3Row::getFieldLong(const char* name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->getLong();
        return -1;
    }

    String Sqlite3Row::getFieldString(const char* name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->m_sValue;
        return NULL;
    }

    const char* Sqlite3Row::getFieldChars(const char* name) {
        AutoLock _l(this);
        sp<NamedValue> nv = getField(name);
        if(nv.has())
            return nv->m_sValue.toChars();
        return NULL;
    }

    // --------------------------------------------------------------

    Sqlite3Response::Sqlite3Response(Sqlite3* sqlite){
        m_sqlite3 = sqlite;
        m_lastMsg = NULL;
    }

    Sqlite3Response::~Sqlite3Response(){
        m_sqlite3 = NULL;
        if( m_lastMsg ){
            sqlite3_free(m_lastMsg);
            m_lastMsg = NULL;
        }
    }

    DFW_PRIVATE
    sp<Retval> Sqlite3Response::query(const char* query){
        if( m_lastMsg ){
            sqlite3_free(m_lastMsg);
            m_lastMsg = NULL;
        }
        
        sqlite3* r_db = (sqlite3*)m_sqlite3->m_db;
        int rc = sqlite3_exec(r_db
                            , query, exec_callback, this, &m_lastMsg);
        if( rc != SQLITE_OK ){
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, rc
                       , "SQL ERROR: rc=%d, %s, query=%s"
                       , rc, m_lastMsg, query);
        }
        return NULL;
    }

    DFW_PRIVATE
    int Sqlite3Response::results(int argc, char **argv, char** cols)
    {
        AutoLock _l(this);
        sp<Retval> retval;
        sp<Sqlite3Row> row = new Sqlite3Row( m_aRows.size() );
        for(int k=0; k<argc; k++){
           sp<NamedValue> nv = new NamedValue(cols[k], argv[k]);
           if( DFW_RET(retval, row->m_aFields.insert(nv)) )
               return (retval->error() ? retval->error() : -1);
        }

        if( DFW_RET(retval, m_aRows.insert(row)) )
           return (retval->error() ? retval->error() : -1);
        return 0;
    }

    DFW_STATIC DFW_PRIVATE
    int Sqlite3Response::exec_callback(void* obj
                                , int argc, char **argv, char** cols)
    {
        sp<Sqlite3Response> resp = ((Sqlite3Response*)obj);
        resp->results(argc, argv, cols);
        return 0;
    }

    // --------------------------------------------------------------

    Sqlite3::Sqlite3(){
        m_db = NULL;
    }

    Sqlite3::~Sqlite3(){
        close();
    }

    sp<Retval> Sqlite3::open(const char* path){
        sp<Retval> retval;
        close_l();
        sqlite3* r_db=NULL;
        int err = sqlite3_open(path, &r_db);
        if( err ) {
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, err
                       , "Not open sqlite3. path=%s", path);
        }
        m_db = (void*)r_db;
        return NULL;
    }

    sp<Retval> Sqlite3::query(sp<Sqlite3Response>& out, const char* query){
        sp<Retval> retval;
        sp<Sqlite3Response> resp = new Sqlite3Response(this);
        if( DFW_RET(retval, resp->query(query)) )
            return DFW_RETVAL_D(retval);
        out = resp;
        return NULL;
    }

    sp<Retval> Sqlite3::close(){
        sp<Retval> retval;
        close_l();
        return NULL;
    }

    void Sqlite3::close_l(){
        if( m_db ) {
            sqlite3* r_db = (sqlite3*)m_db;
            sqlite3_close(r_db);
            m_db = NULL;
        }
    }

};

