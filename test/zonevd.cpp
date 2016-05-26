#include <dframework/fscore/FuseMount.h>
#include <dframework/fscore/SampleWorker.h>

using namespace dframework;

int main(void)
{
    sp<Retval> retval;

    sp<FuseMount>  mount  = new FuseMount();
    sp<SampleWorker>   worker = new SampleWorker();
    sp<FsNodeList> nodelist = new FsNodeList();
    sp<SampleWorkerList> workerlist = new SampleWorkerList();
    sp<FuseRequestList> reqlist = new FuseRequestList();

    worker->setMount(mount);
    worker->setNodeList(nodelist);
    worker->setWorkerList(workerlist);
    worker->setRequestList(reqlist);

    if( DFW_RET(retval, worker->start("/opt/fuse-mount")) ){
        printf("%s\n", retval->dump().toChars() );
        return 1;
    }

    workerlist->joinAll();

    return 0;
};

