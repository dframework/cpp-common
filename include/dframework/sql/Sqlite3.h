#ifndef DFRAMEWORK_SQL_SQLITE3_H
#define DFRAMEWORK_SQL_SQLITE3_H

#include <dframework/base/Retval.h>
#include <dframework/util/ArraySorted.h>
#include <dframework/util/NamedValue.h>

#ifdef __cplusplus
namespace dframework {

    class Sqlite3;
    class Sqlite3Response;

    class Sqlite3Row : public Object
    {
    private:
        unsigned m_no;
        ArraySorted<NamedValue> m_aFields;

    public:
        Sqlite3Row(unsigned no);
        virtual ~Sqlite3Row();

        int size();

        sp<NamedValue> getField(int position);
        int getFieldInt(int position);
        long getFieldLong(int position);
        String getFieldString(int position);
        const char* getFieldChars(int position);

        sp<NamedValue> getField(String& name);
        int getFieldInt(String& name);
        long getFieldLong(String& name);
        String getFieldString(String& name);
        const char* getFieldChars(String& name);

        sp<NamedValue> getField(const char* name);
        int getFieldInt(const char* name);
        long getFieldLong(const char* name);
        String getFieldString(const char* name);
        const char* getFieldChars(const char* name);

        DFW_OPERATOR_EX_DECLARATION(Sqlite3Row, m_no);

        friend class Sqlite3Response;       
 
    };

    // --------------------------------------------------------------

    class Sqlite3Response : public Object
    {
    private:
        sp<Sqlite3> m_sqlite3;
        char*       m_lastMsg;
        ArraySorted<Sqlite3Row> m_aRows;

    public:
        Sqlite3Response(Sqlite3* sqlite);
        virtual ~Sqlite3Response();

        inline int size() { return m_aRows.size(); }
        inline sp<Sqlite3Row> getRow(int k) { return m_aRows.get(k); }

        friend class Sqlite3;

    private:
        sp<Retval> query(const char* query);
        int results(int argc, char** argv, char**);
        static int exec_callback(void *, int argc, char** argv, char**);

    };

    class Sqlite3 : public Object
    {
    private:
        void* m_db; //sqlite3* m_db;

    public:
        Sqlite3();
        virtual ~Sqlite3();

        sp<Retval> open(const char* path);
        inline sp<Retval> open(String& path){
            return open(path.toChars());
        }

        sp<Retval> query(sp<Sqlite3Response>& out, const char* q);
        inline sp<Retval> query(sp<Sqlite3Response>& out, String& q){
            return query(out, q.toChars());
        }

        sp<Retval> close();

        friend class Sqlite3Response;

    private:
        void close_l();

    };

};
#endif

#endif /* DFRAMEWORK_SQL_SQLITE3_H */

