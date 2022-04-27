#include <list>
#include <time.h>

#include "Event.h"

#include <assert.h>
#include <vector>
#include <sys/time.h>
#include <errno.h>

namespace Gengine {
namespace Multithreading {
Event::Event()
          :m_bManualReset(false)
          ,m_bSignaled(false)
{
}

Event::~Event()
{
}

void Event::Create(bool bManualReset, bool bInitialState)
{
    // set initial setting
    m_bManualReset = bManualReset;
    m_bSignaled = bInitialState;
}

void Event::Set()
{
    std::lock_guard<std::mutex> csMutexContidions(m_csMutexConditions);
    std::list<mutex_cond_t*>::iterator it;
    for(it=m_MutexConditions.begin();it!=m_MutexConditions.end();++it)
    {
       /*
         * All the mutexes on the Event Handle List are first
         * locked, then all condition variables are signalled
         * and then all mutexes are unlocked. This is done to
         * prevent any race condition.
         */
        pthread_mutex_lock(&(*it)->m_Mutex);
    }
    m_bSignaled=true;
    for(it=m_MutexConditions.begin();it!=m_MutexConditions.end();++it)
    {
        pthread_cond_signal(&(*it)->m_Cond);
        if(!m_bManualReset)
            break;//awake just only one thread
    }
    for(it=m_MutexConditions.begin();it!=m_MutexConditions.end();++it)
    {
       pthread_mutex_unlock( &(*it)->m_Mutex);
    }
}

void Event::Reset()
{
   m_bSignaled = false;
}
//returns true if wait successfull or false if wait
//exited by timeout
//if dwMilliseconds==INFINITE
//thread will wait finfinite for the event
bool Event::Wait(std::uint32_t dwMilliseconds)
{
    Event* pThis=this;
    return WaitMultiple(&pThis,1,true,dwMilliseconds)==0;
}
//returns number of event being setted or -1 if wait timeouted

int Event::WaitMultiple(Event** ppEvents, int iEventsCount,
                                  bool bWaitAll, std::uint32_t dwMilliseconds)
{
    bool Result;
    int i;
    struct timespec to_time; /* time value */
    //struct timespec delta_time; /* time value */
    if (dwMilliseconds !=Event::WAIT_INFINITE)
    {
        timeval now_;
        gettimeofday(&now_, 0);
        to_time.tv_sec = now_.tv_sec + dwMilliseconds / 1000;
        std::uint32_t uSec=(dwMilliseconds % 1000) * 1000;//numer of microseconds besides seconds
        uSec+=now_.tv_usec;
        to_time.tv_sec+=uSec/1000000;
        to_time.tv_nsec = (uSec % 1000000) * 1000;
    }
    if (bWaitAll)
    {
        Result = true;
        std::list<mutex_cond_t*> lstMutexCond;
        //add separame mutex_cond to each event list
        for (i = 0; i < iEventsCount; ++i)
        {
            Event *event = ppEvents[i];
            mutex_cond_t *m=new mutex_cond_t;
            pthread_mutex_init(&m->m_Mutex, NULL);
            pthread_cond_init(&m->m_Cond, NULL);
            event->AddMutexCond(m);
            lstMutexCond.push_back(m);
        }
        /* Wait on each event to be set or timedout */
        std::list<mutex_cond_t*>::iterator it;
        it=lstMutexCond.begin();
        for (i = 0; i < iEventsCount; ++i)
        {
            Event *event = ppEvents[i];
            mutex_cond_t *m=*it;
            ++it;
            pthread_mutex_lock(&m->m_Mutex);
            while (!event->m_bSignaled)
            {
                if (dwMilliseconds != Event::WAIT_INFINITE)
                {
                    int iResult=pthread_cond_timedwait(&m->m_Cond,&m->m_Mutex, &to_time);
                    if (iResult == ETIMEDOUT)
                    {
                        Result = false;
                        break;
                    }
                }
                else
                {
                    pthread_cond_wait(&m->m_Cond,&m->m_Mutex);
                }
            }
            if(!event->m_bManualReset)
            {
                //reset this autoreset event
                //NOTE: all other threads which will try set this event must 
                //lock m->m_Mutex before modifying m_bSignaled,
                //so we can safely modify m_bSignaled here, till we own m->m_Mutex
                event->m_bSignaled=false;
            }
            pthread_mutex_unlock(&m->m_Mutex);
            if (Result == false)
            {//timeout occured
                break;
            }
        }
        /*
         * Delete all the List elements created by this thread for
         * each event.
         */
        it=lstMutexCond.begin();
        for (i = 0; i < iEventsCount; ++i)
        {
            mutex_cond_t *m=*it;
            ++it;
            Event *event = ppEvents[i];
            event->RemoveMutexCond(m);
            pthread_mutex_destroy(&m->m_Mutex);
            pthread_cond_destroy(&m->m_Cond);
            delete m;
        }
        if(Result)
            return 0;//success - all events has been acquired
        return -1;//timeout
    }
    else
    {
        mutex_cond_t *pCommonMutexCond=new mutex_cond_t;
        Result = false;
        int iEvent=-1;//number of the signaled event
        //one common MutexCond object is added to all event, to which we'll try wait
        pthread_mutex_init(&pCommonMutexCond->m_Mutex, NULL);
        pthread_cond_init(&pCommonMutexCond->m_Cond, NULL);
        for (i = 0; i < iEventsCount; ++i)
        {
            Event *event = ppEvents[i];
            event->AddMutexCond(pCommonMutexCond);
        }
        //wait for event
        pthread_mutex_lock(&pCommonMutexCond->m_Mutex);
        for (i = 0; i < iEventsCount; ++i)
        {
            Event *event = ppEvents[i];
            if (event->m_bSignaled)
            {//success!
                Result = true;
                iEvent=i;
                break;
            }
        }
        if (Result == false)
        {//there are no signaled event. wait some time...
            if (dwMilliseconds != Event::WAIT_INFINITE)
            {
                if (pthread_cond_timedwait(&pCommonMutexCond->m_Cond,
                                            &pCommonMutexCond->m_Mutex,
                                            &to_time) != ETIMEDOUT)
                {
                    //yes, some evbent is signaled!
                    Result = true;
                }
            }
            else
            {//infinite wauit for some event
                pthread_cond_wait(&pCommonMutexCond->m_Cond,
                                  &pCommonMutexCond->m_Mutex);
                Result = true;
            }
            if(Result)
            {
                //find index of the signaled event
                //NOTE: next operations are safe because of
                //other threads to sygnal event must lock
                //all mutexes in that event list, includeing
                //out pCommonMutex; but, because of we're holding this mutex, tay will
                //wait until we're done
                for(i=0;i<iEventsCount;++i)
                {
                    if(ppEvents[i]->m_bSignaled)
                    {
                        iEvent=i;
                        if(!ppEvents[i]->m_bManualReset)
                            //reset this auto-reset event
                            ppEvents[i]->m_bSignaled=false;
                        break;
                    }
                }
                assert(iEvent>=0);//if none of the events are signaled, why we're awaked?
            }
        }
        pthread_mutex_unlock(&pCommonMutexCond->m_Mutex);
        /*
         * Delete all the List elements created by this thread for
         * each event.
         */
        for (i = 0; i < iEventsCount; ++i)
        {
            Event *event = ppEvents[i];
            event->RemoveMutexCond(pCommonMutexCond);
        }
        pthread_mutex_destroy(&pCommonMutexCond->m_Mutex);
        pthread_cond_destroy(&pCommonMutexCond->m_Cond);
        delete pCommonMutexCond;
        return iEvent;
    }
    //how we're got here?
    assert(0);
}

void Event::AddMutexCond(mutex_cond_t* pMutexCond)
{
    std::lock_guard<std::mutex> csMutexContidions(m_csMutexConditions);
    m_MutexConditions.push_back(pMutexCond);
}

void Event::RemoveMutexCond(mutex_cond_t* pMutexCond)
{
    std::lock_guard<std::mutex> csMutexContidions(m_csMutexConditions);
    m_MutexConditions.remove(pMutexCond);
}
}
}
