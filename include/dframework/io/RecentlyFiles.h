#ifndef DFRAMEWORK_IO_RECENTLYFILES_H
#define DFRAMEWORK_IO_RECENTLYFILES_H

#include <dframework/base/Retval.h>
#include <dframework/util/Array.h>
#include <dframework/util/ArraySorted.h>

#ifdef __cplusplus
namespace dframework {

    class RecentlyFiles;

    class RecentlyFiles : public Object
    {
    public:
        class Time : public Object
        {
        public:
            DFW_DEFINITION_SAFE_COUNT(Time);

            sp<String> m_sName;
            dfw_time_t m_iTime;
            sp<Object> m_context;

        public:
            Time(sp<String>& name, dfw_time_t time, sp<Object>& context);
            Time(sp<String>& name, dfw_time_t time);
            virtual ~Time();

            DFW_OPERATOR_EX_DECLARATION(Time, m_iTime);

            friend class RecentlyFiles;
        };

        class Name : public Object
        {
        public:
            DFW_DEFINITION_SAFE_COUNT(Name);

            sp<String>              m_sName;
            sp<RecentlyFiles::Time> m_oTime;

        public:
            Name(sp<String>& name, sp<RecentlyFiles::Time>& time);
            Name(sp<String>& name);
            virtual ~Name();

            DFW_OPERATOR_EX_DECLARATION(Name, m_sName);

            friend class RecentlyFiles;
        };

    public:
        DFW_DEFINITION_SAFE_COUNT(RecentlyFiles);

        ArraySorted<Time> m_aTimes;
        ArraySorted<Name> m_aNames;

    public:
        RecentlyFiles();
        virtual ~RecentlyFiles();

        sp<Time> getTimeNode(int position);

        int size();

        bool hasRecently(const char* name);
        inline bool hasRecently(const String& name){
            return hasRecently(name.toChars());
        }

        sp<Retval> add(const char* name, dfw_time_t time, sp<Object>&);
        inline sp<Retval> add(const String& name, dfw_time_t time
                            , sp<Object>& ctx){
            return add(name.toChars(), time, ctx);
        }

        sp<Retval> remove(const char* name);
        inline sp<Retval> remove(const String& name){
            return remove(name.toChars());
        }

    private:
        sp<Retval> updateTime(sp<String>& name
                            , dfw_time_t old, dfw_time_t val);
        sp<Retval> removeTime(sp<Time>& fTime);

    };

};
#endif

#endif /* DFRAMEWORK_IO_RECENTLYFILES_H */

