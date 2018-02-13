//
//  SimpleOSCThreaded.hpp
//
//
//  Created by James on 30/01/2018.
//  Copyright © 2018 James. All rights reserved.
//

#ifndef SimpleOSCThreaded_h
#define SimpleOSCThreaded_h

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <memory.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include "RingBufferAny.hpp"

class SimpleOSCThreaded
{
public:
    // Gets rid of thread
    ~SimpleOSCThreaded ( void )
    { // Stop thread
        if ( running.load(  ) )
        {
            running.store( false );
            processMainThread.join(  );
        }
    }
    
    // Spawn thread ( returns true on success )
    bool setup ( size_t eventQueueSize = 4096, useconds_t threadSleepTime = 500000, std::string mainAddress = "/main/" )
    {
        this->eventQueue.setup( eventQueueSize ); // Some space for messages
        this->mainAddress = mainAddress;
        
        if ( !running.load(  ) )
        {
            this->threadSleepTime = threadSleepTime;
            // Spawn process thread
            running.store( true );
            rebind.store( false );
            processMainThread = std::thread( &SimpleOSCThreaded::mainLoop, this );
            return true;
        }
        return false;
    }
    
    void setHostAndPort( std::string host, int32_t port )
    {
        connectionLock.lock(  );
        this->host = host;
        this->port = port;
        connectionLock.unlock(  );
        rebind.store( true );
    }
    
    // Add event to the send queue
    void triggerEvent ( std::string val = "" )
    {
        std::ostringstream sendBuffer;
        sendBuffer << padInput( mainAddress ) << padInput( ",s" ) + padInput( val );
        // Message as string
        std::string sendBufferString = padInput( sendBuffer.str(  ) );
        
        // Add to queue
        while ( !eventQueue.push( const_cast< char * >( sendBufferString.c_str(  ) ), sendBufferString.length(  ) ) )
        {
            // Double the size of the sendBuffer if message doesn't fit ( not clean and the allocator does block! )
            eventQueue.reset(  );
        }
    }
    
protected:
    std::string padInput ( std::string input )
    {
        input += '\0';
        while ( input.length(  ) % 4 > 0 )
            input += '\0';
        return input;
    }

    bool resolveConnection ( void )
    {
        //create socket
        sockfd = socket( AF_INET, SOCK_DGRAM, 0 );
        if ( sockfd == -1)
            return false;

        //assign local values
        connectionLock.lock(  );
        server.sin_addr.s_addr = inet_addr( host.c_str(  ) );
        server.sin_family = AF_INET;
        server.sin_port = htons( port );
        connectionLock.unlock(  );
        return connect( sockfd, (struct sockaddr *) &server, sizeof( server ) ) >= 0;
    }
    
    bool sendMessage ( const char* message, size_t messageLength )
    {
        size_t sent, received;
        long bytes;

        bytes = sendto( sockfd, message, messageLength, NULL, ( sockaddr* ) & server, sizeof( server ) );
        if ( bytes < 0 )
            return false;

        return true;
    }

    size_t closestMultipleOfFour ( size_t number )
    {
        if ( number % 4 == 0 )
            return number;
        return ( ( number / 4 ) + 1 ) * 4;
    }
    
    void mainLoop ( void )
    {
        while ( running.load(  ) )
        {
            RingBufferAny::VariableHeader popHeader;
            while ( eventQueue.anyAvailableForPop( popHeader ) )
            {   // See if can connect
                while ( !canConnect || rebind.load(  ) )
                {
                    canConnect = resolveConnection(  );
                    if ( canConnect )
                        rebind.store( false );
                    usleep( 500000 );
                }
                // Get message from queue
                char message[ popHeader.valuesPassed ];
                eventQueue.pop( message, popHeader.valuesPassed );
                //__android_log_print( ANDROID_LOG_WARN, "resettingconnection", "bla %d %d %s", mFour, popHeader.valuesPassed, message );
                
                // Send message
                while ( !sendMessage( message, popHeader.valuesPassed ) )
                    canConnect = false;
            }
            // Put the thread to sleep for x time
            usleep( threadSleepTime );
        }
        close( sockfd );
    }
    
private:
    bool canConnect = false;
    
    int32_t sockfd;
    struct sockaddr_in server;
    
    std::string mainAddress;
    std::string host = "localhost";
    int32_t port = 8888;
    Spinlock connectionLock;
    
    std::thread processMainThread;
    std::atomic< bool > running;
    std::atomic< bool > rebind;
    
    RingBufferAny eventQueue;
    
    
    useconds_t threadSleepTime; // 500ms sleep time ( event calls are slow anyway )
};

#endif /* SimpleOSCThreaded_h */
