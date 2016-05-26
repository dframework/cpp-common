#include <dframework/ssh2/SSH2Global.h>

#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

namespace dframework {

    DFW_STATIC
    sp<SSH2Global> SSH2Global::m_ssh2_global;

    DFW_STATIC
    Object SSH2Global::m_ssh2_safe;

    DFW_STATIC
    sp<Retval> SSH2Global::instance(sp<SSH2Global>& out){
        sp<Retval> retval;
        AutoLock _l(&m_ssh2_safe);
        if(!m_ssh2_global.has()){
            sp<SSH2Global> global = new SSH2Global();
            if( DFW_RET(retval, global->ready()) )
                return DFW_RETVAL_D(retval);
            m_ssh2_global = global;
        }
        out = m_ssh2_global;
        return NULL;
    }

    SSH2Global::SSH2Global(){
    }

    SSH2Global::~SSH2Global(){
        AutoLock _l(&m_ssh2_safe);
        libssh2_exit();
    }

    sp<Retval> SSH2Global::ready(){
        int rc;
        if( (rc = libssh2_init(0)) )
            return DFW_RETVAL_NEW_MSG(DFW_ERROR, 0
                   , "Not libssh2_init, rc=%d", rc);
        return NULL;
    }

};

