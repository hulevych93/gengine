#include "LinuxSingleInstanceRegistrator.h"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/stat.h>

#include <core/Encoding.h>
#include <core/Logger.h>

namespace Gengine {
namespace InterprocessSynchronization {
LinuxSingleInstanceRegistrator::LinuxSingleInstanceRegistrator(std::wstring&& objectName)
: InstanceRegistratorInterface(std::move(objectName))
, m_iSingleInstanceFile(-1)
{}

LinuxSingleInstanceRegistrator::~LinuxSingleInstanceRegistrator()
{
    if(IsInstanceRegistered())
    {
        UnregisterInstance();
    }
}

bool LinuxSingleInstanceRegistrator::RegisterInstance()
{
    //we may run under the root, set mode of files to 777
    //to allow anybody other run after us
    umask(0000);
    if(m_iSingleInstanceFile != -1)
        return true;//if we've successfully created this file this means that no other instances running
    int iFile=open(toUtf8(GetObjectName()).c_str(), O_CREAT|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);
    if(iFile==-1)
    {
        GLOG_ERROR("Failed open file; erro %d",errno);
        assert(0);
        return false;//some error occured - treat is as another instance is running
    }
    //write something to file to ensure we're have data to lock
    char uDummy='x';
    write(iFile,&uDummy,1);
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=1;
    lock.l_type=F_WRLCK;
    if(fcntl(iFile,F_SETLK,&lock)==-1)
    {//we cannot acuire the lock - another instance is runing
        close(iFile);
        return false;
    }
    m_iSingleInstanceFile=iFile;
    //we've successfully acuired the lock;
    //this is first instance
    return true;
}

void LinuxSingleInstanceRegistrator::UnregisterInstance()
{
    if(m_iSingleInstanceFile==-1)
        return;
    struct flock lock;
    memset(&lock,0,sizeof(lock));
    lock.l_whence=SEEK_SET;
    lock.l_start=0;
    lock.l_len=1;
    lock.l_type=F_UNLCK;
    fcntl(m_iSingleInstanceFile,F_SETLK,&lock);
    close(m_iSingleInstanceFile);
    m_iSingleInstanceFile=-1;
}

bool LinuxSingleInstanceRegistrator::IsInstanceRegistered() const
{
    return m_iSingleInstanceFile != -1;
}
}
}
