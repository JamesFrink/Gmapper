//
//  ThreadedProcess.h
//
//  Created by James on 19/11/2017.
//
//

#ifndef ThreadedProcess_h
#define ThreadedProcess_h

#include <stdio.h>
#include <stdint.h>
#include <functional>
#include <thread>
#include <atomic>
#include <unistd.h>

class ThreadedProcess {
public:
    ThreadedProcess ( void )
    {
        running.store( false );
    }
    
    virtual ~ThreadedProcess ( void )
    {
        stopThread(  );
    }
    
    bool isThreadRunning ( void ) const
    {
        return running.load(  );
    }
    
    // ( re )Start main thread
    void startThread ( uint64_t threadSleepTime = 1 )
    {
        this->threadSleepTime = threadSleepTime;
        if ( running.load(  ) )
        { // Thread is already running
            stopThread(  );
        }
        running.store( true );
        processMainThread = std::thread( &ThreadedProcess::mainLoop, this ); // Spawn thread
    }
    
    // Stop main thread
    void stopThread ( void )
    {
        if ( running.load(  ) )
        {
            running.store( false );
            processMainThread.join(  );
        }
    }
    
protected:
    virtual void mainThreadCallback ( void ) = 0;
    
    std::thread processMainThread;
    
private:
    void mainLoop ( void )
    {
        while ( running.load(  ) )
        {
            mainThreadCallback(  );
            usleep( threadSleepTime );
        }
    }
    
    std::atomic< bool > running;
    uint64_t threadSleepTime = 1;
};


#endif /* ThreadedProcess_h */
