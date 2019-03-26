#ifndef DFRAMEWORK_NET_URI_H
#define DFRAMEWORK_NET_URI_H
#define DFRAMEWORK_NET_URI_VERSION 1

#include <dframework/base/Retval.h>
#include <dframework/lang/String.h>
#include <dframework/util/Regexp.h>
#include <dframework/util/Array.h>

#ifdef __cplusplus
namespace dframework {

    class URI : public Object
    {
    public:
        class PathInfo : public Object {
        public:
            String        m_sPath;
            String        m_sWinPath;
            String        m_sPrefixPath;
            String        m_sWinPrefixPath;
            String        m_sName;
            Array<String> m_aPaths;

            inline PathInfo() {}
            inline PathInfo(const char* path) : Object::Object() {
                parse(path); 
            }
            inline PathInfo(const char* path, int len) : Object::Object() {
                parse(path, len); 
            }
            inline PathInfo(const String& path) : Object::Object() { 
                parse(path); 
            }
            inline PathInfo(const PathInfo& path) : Object::Object() {
                parse(path.m_sPath); 
            }
            inline virtual ~PathInfo() {}

            void parse(const char* path, int len);
            inline void parse(const char* path) { 
                parse(path, (path ? strlen(path):0));
            }
            inline void parse(const String& path) { 
                parse(path.toChars(), path.length());
            }

            // FIXME: This is deprecate.
            inline void set(const char *path, int len){ parse(path, len); }

            inline int size() const { return m_aPaths.size(); }
            String path(int no) const;
            String fullpath(int no) const;
            inline const String& path() const { return m_sPath; }
            inline const String& winPath() const { return m_sWinPath; }
        };

        class FileInfo : public Object {
        public:
            bool   m_bUse;
            String m_sDirname;
            String m_sFilename;
            String m_sName;
            String m_sExtension;

            FileInfo();
            inline virtual ~FileInfo() {}

            void clean();
            void parse(const char *path);

            // FIXME: This is deprecate
            inline void set(const char *path) { parse(path); }

            String path();
            String winPath();

            FileInfo& operator=(const char *path);
            FileInfo& operator=(const FileInfo &info);
        };

        class Attr : public Object
        {
        public:
            int m_type;
            int m_integer;
            String m_name;
            String m_value;

        public:
            Attr(int type);
            Attr(int type, int value);
            Attr(int type, const char* value);
            Attr(int type, const char* name, const char* value);
            virtual ~Attr();

            DFW_OPERATOR_EX_DECLARATION(Attr, m_type);
        };

    private:
        String m_sUri;
        String m_sScheme;
        String m_sUser;
        String m_sPass;
        String m_sHost;
        int    m_iPort;
        String m_sPath;
        String m_sQuery;
        String m_sFragment;

        Array<Attr> m_aAttrs;

        sp<Retval> ___parseHostAndPath(const String&);
        sp<Retval> ___parseHostAndPath(const char*);
        //void ___parse_HP_FileScheme(
        //      Regexp& regexp, int last
        //    , const char* str
        //    , String& host, String& slash, String& path);
        //void ___parse_UP_HP(Regexp&, const char*);
        //void ___parse_HP(Regexp&, const char*);
        //void ___parse_account(const String& str);
        //void ___parse_host(const char *str);
        //void ___parse_path(const char *str);

    public:
        URI();
        URI(const char* uri);
        URI(const String& uri);
        URI(const URI& uri);
        URI(sp<URI>& uri);
        virtual ~URI();
        virtual String toString() const;

        void clear();
        void set(const URI& uri);
        void set(sp<URI>& uri);

        sp<Retval> parse(const char*);
        sp<Retval> parse(const String&);

        inline const String& getUri() const { return m_sUri; }
        inline const String& getScheme() const { return m_sScheme; }
        inline const String& getUser() const { return m_sUser; }
        inline const String& getPass() const { return m_sPass; }
        inline const String& getHost() const { return m_sHost; }
        inline       int     getPort() const {
            return (m_iPort!=0 ? m_iPort : getDefaultPort());
        }
        inline       int     getRealPort() const { return m_iPort; }
        inline       int     getDefaultPort() const {
            return getDefaultPort(m_sScheme);
        }
        inline const String& getPath() const { return m_sPath; }
        inline const String& getQuery() const { return m_sQuery; }
        inline const String& getFragment() const { return m_sFragment; }

        inline void setScheme(const char* p)  { m_sScheme = p; }
        inline void setUser(const char* p)    { m_sUser = p; }
        inline void setPass(const char* p)    { m_sPass = p; }
        inline void setHost(const char* p)    { m_sHost = p; }
        inline void setPort(const int p)      { m_iPort = p; }
        inline void setPath(const char* p)    { m_sPath = p; }
        inline void setQuery(const char* p)   { m_sQuery = p; }
        inline void setFragment(const char* p){ m_sFragment = p; }

        sp<Retval> setAttribute(int type, int value);
        sp<Retval> setAttribute(int type, const char* value);
        sp<Retval> setAttribute(int type
                              , const char* name, const char* value);

        int sizeAttribute() { return m_aAttrs.size(); }
        sp<Attr> getAttribute(int pos) { return m_aAttrs.get(pos); }

        static int getDefaultPort(const char*, int defaultPort=80);
        static int getDefaultPort(const String&, int defaultPort=80);

        URI& operator=(const char* uri);
        URI& operator=(const String& uri);
        URI& operator=(const URI& uri);
        URI& operator=(sp<URI>& uri);

        DFW_OPERATOR_EXP_DECLARATION(URI);

    };

};
#endif

#endif /* DFRAMEWORK_NET_URI_H */

