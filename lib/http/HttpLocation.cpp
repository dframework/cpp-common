#include <dframework/http/HttpLocation.h>
#include <dframework/net/Hostname.h>

namespace dframework {


    HttpLocation::HttpLocation(){
    }

    HttpLocation::~HttpLocation(){
    }

    sp<Retval> HttpLocation::parse(
            URI& outUri, URI& orgUri, String& sLocation, const char *orgip)
    {
        sp<Retval> retval;

        if( sLocation.empty() )
            return DFW_RETVAL_NEW(DFW_E_NOTFINDLOCATION,0);

        const char *loc = sLocation.toChars();
        if('/'==loc[0]){
            outUri = orgUri;
            outUri.setPath(loc);
            return NULL;
        }

        if(sLocation.indexOf("http://")==0){
            URI test = loc;
            if( test.getHost().equals(orgUri.getHost().toChars()) 
                    && test.getPort() == orgUri.getPort() ){
                outUri = orgUri;
                outUri.setPath(test.getPath().toChars());
                outUri.setQuery(test.getQuery().toChars());
                outUri.setFragment(test.getFragment().toChars());
                return NULL;
            }

            bool samip = false;
            Hostname hostname;
            if(DFW_RET(retval, hostname.get(loc)))
                return DFW_RETVAL_D(retval);
            int hsize = hostname.size();
            if(0==hsize)
                return DFW_RETVAL_NEW(DFW_E_HOST_NOT_FOUND,0);

            for(int hk=0; hk<hsize; hk++){
                sp<Hostname::Result> hr = hostname.getResult(hk);
                if( hr->m_sIp.equals(orgip) ){
                    samip = true;
                    break;
                }
            }

            if( (!samip) || (test.getPort()!=orgUri.getPort()) ){
                outUri = loc;
                /*if( (status=m_pClient->connect_real()) ){
                    return status;
                }*/
                return DFW_RETVAL_NEW(DFW_T_RECONNECT,0);
            }else{
                outUri = loc;
            }

            return NULL;
        }

        int index = orgUri.getPath().lastIndexOf("/");
        if(index==-1){
            return DFW_RETVAL_NEW(DFW_ERROR,0);
        }

        String npath;
        npath.set(orgUri.getPath().toChars(), index+1);
        npath.append(loc, sLocation.length());
        outUri.setPath(npath.toChars());

        return NULL;
    }

};

