/*
  Copyright (c) 2004-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "gloox.h"

#include "connectiontcpbase.h"
#include "dns.h"
#include "logsink.h"
#include "prep.h"
#include "mutexguard.h"
#include "util.h"

#if !defined( _WIN32 ) && !defined( _WIN32_WCE )
# include <arpa/inet.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/select.h>
# include <netinet/in.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <netdb.h>
#endif

#if defined( _WIN32 ) || defined( __MINGW32__ )
# include <winsock2.h>
# include <ws2tcpip.h>
typedef int socklen_t;
#endif

#include <ctime>

#include <cstdlib>
#include <string>

namespace gloox
{

  ConnectionTCPBase::ConnectionTCPBase( const LogSink& logInstance,
                                        const std::string& server, int port )
    : ConnectionBase( 0 ),
      m_logInstance( logInstance ), m_buf( 0 ), m_socket( -1 ), m_totalBytesIn( 0 ),
      m_totalBytesOut( 0 ), m_bufsize( 8192 ), m_cancel( true )
  {
    init( server, port );
  }

  ConnectionTCPBase::ConnectionTCPBase( ConnectionDataHandler* cdh, const LogSink& logInstance,
                                        const std::string& server, int port )
    : ConnectionBase( cdh ),
      m_logInstance( logInstance ), m_buf( 0 ), m_socket( -1 ), m_totalBytesIn( 0 ),
      m_totalBytesOut( 0 ), m_bufsize( 8192 ), m_cancel( true )
  {
    init( server, port );
  }

  void ConnectionTCPBase::init( const std::string& server, int port )
  {
// FIXME check return value?
    prep::idna( server, m_server );
    m_port = port;
    m_buf = static_cast<char*>( calloc( m_bufsize + 1, sizeof( char ) ) );
  }

  ConnectionTCPBase::~ConnectionTCPBase()
  {
    cleanup();
    free( m_buf );
    m_buf = 0;
  }

  void ConnectionTCPBase::disconnect()
  {
    util::MutexGuard rm( m_recvMutex );
    m_cancel = true;
  }

  bool ConnectionTCPBase::dataAvailable( int timeout )
  {
    if( m_socket < 0 )
      return true; // let recv() catch the closed fd

    fd_set fds;
    struct timeval tv;

    FD_ZERO( &fds );
    // the following causes a C4127 warning in VC++ Express 2008 and possibly other versions.
    // however, the reason for the warning can't be fixed in gloox.
    FD_SET( m_socket, &fds );

    tv.tv_sec = timeout / 1000000;
    tv.tv_usec = timeout % 1000000;

    if( FD_ISSET( m_socket, &fds ) != 0 )
    {
      return ( select( m_socket + 1, &fds, 0, 0, timeout == -1 ? 0 : &tv ) > 0 );
    }
    else
    {
      return false;
    }
    /*
        return ( ( select( m_socket + 1, &fds, 0, 0, timeout == -1 ? 0 : &tv ) > 0 )
             && FD_ISSET( m_socket, &fds ) != 0 );
    */
  }

  ConnectionError ConnectionTCPBase::receive()
  {
    if( m_socket < 0 )
      return ConnNotConnected;

    ConnectionError err = ConnNoError;
    while( !m_cancel && ( err = recv( 1000000 ) ) == ConnNoError )
      ;
    return err == ConnNoError ? ConnNotConnected : err;
  }

  /**
  * @brief Indicates whether a socket is ready for reading/writing.
  * @param readop Indicates whether this is a read operation (@b true) or not (@b false, write operation).
  * @return Returns 1 if socket is ready, 0 if timed out, else -errno.
  *
  */
  int ConnectionTCPBase::waitForSocketReady( bool readop )
  {
    struct timeval tv, *tvptr;
    fd_set fdset;
    int sres;

    int timeout = this->timeout();
    int lastError;

    do
    {
        FD_ZERO( &fdset );
        FD_SET( m_socket, &fdset );
        if( timeout < 0 )
        {
          tvptr = 0;
        }
        else
        {
          tv.tv_sec = timeout / 1000;
          tv.tv_usec = ( timeout % 1000 ) * 1000;
          tvptr = &tv;
        }
        sres = select( m_socket + 1,
                       readop ? &fdset : 0,
                       readop ? 0 : &fdset,
                       0,
                       tvptr );
#if defined( _WIN32 )
        lastError = WSAGetLastError();
      } while( sres == -1 && lastError == WSAEINTR );
#else
        lastError = errno;
      } while( sres == -1 && lastError == EINTR );
#endif

    // Timeout
    if( sres == 0 )
    {
        return 0;
    }
    else if( sres < 0 )
    {
        return -lastError;
    }
    return 1;
  }

  bool ConnectionTCPBase::send( const std::string& data )
  {
    if( data.empty() )
      return false;

    return send( data.c_str(), data.length() );
  }

  bool ConnectionTCPBase::send( const char* data, const size_t length )
  {
    m_sendMutex.lock();

    if( !length || !data || ( m_socket < 0 ) )
    {
        m_sendMutex.unlock();
        return false;
    }

    int sent = 0;
    size_t num = 0;
    size_t len = length;
    size_t count = 0; // number of send failures

    do
    {
      sent = static_cast<int>( ::send( m_socket, data + num, static_cast<int>( len - num ), 0 ) );
      if( sent > 0 )
        num += sent;
    } while( num < len && sent != -1 );

    // Non-blocking (timeout value !=1) socket processing
#if defined( _WIN32 )
    int lastError = WSAGetLastError();
    if (sent == -1 && lastError == WSAEWOULDBLOCK  && timeout() != -1)
#else
    int lastError = errno;
    if( sent == -1 && ( lastError == EAGAIN || lastError == EWOULDBLOCK ) && timeout() != -1 )
#endif
    {
      // Wait for socket
      int waitStatus = waitForSocketReady( false );

      while( waitStatus != 1 && count < 5 )
      {
        m_logInstance.err( LogAreaClassConnectionTCPBase, "ConnectionTCPBase::send(): retrying to wait socket, waitstatus="
                              + util::int2string( waitStatus ) + " retry number=" + util::int2string( static_cast<int>( count ) ) );

        waitStatus = waitForSocketReady( false );
        count++;
      }

      // Socket is ready now, we send packets
      if( waitStatus == 1 )
      {
        do
        {
          sent = static_cast<int>( ::send( m_socket, data + num, static_cast<int>( len - num ), 0 ) );
          if( sent > 0 )
            num += sent;
        } while( num < len && sent != -1 );
      }
      else
      {
        m_logInstance.err( LogAreaClassConnectionTCPBase, "ConnectionTCPBase::send(): Socket unavailable, lastError="
                              + util:: int2string( lastError ) + " waitstatus=" + util::int2string( waitStatus )
                              + " number of retries=" + util::int2string( static_cast<int>( count ) ) );
      }
    }

    m_totalBytesOut +=num;

    m_sendMutex.unlock();

    if( sent == -1 )
    {
      // send() failed for an unexpected reason
      std::string message = "send() failed. "
#if defined( _WIN32 )
        "WSAGetLastError: " + util::int2string( lastError );
#else
        "errno: " + util::int2string( lastError ) + ": " + strerror( lastError );
#endif
      m_logInstance.err( LogAreaClassConnectionTCPBase, message );

      if( m_handler )
        m_handler->handleDisconnect( this, ConnIoError );
    }

    return sent != -1;
  }

  void ConnectionTCPBase::getStatistics( long int &totalIn, long int &totalOut )
  {
    totalIn = m_totalBytesIn;
    totalOut = m_totalBytesOut;
  }

  void ConnectionTCPBase::cleanup()
  {
    if( !m_sendMutex.trylock() )
      return;

    if( !m_recvMutex.trylock() )
    {
      m_sendMutex.unlock();
      return;
    }

    if( m_socket >= 0 )
    {
      DNS::closeSocket( m_socket, m_logInstance );
      m_socket = -1;
    }

    m_state = StateDisconnected;
    m_cancel = true;
    m_totalBytesIn = 0;
    m_totalBytesOut = 0;

    m_recvMutex.unlock(),
    m_sendMutex.unlock();
  }

  int ConnectionTCPBase::localPort() const
  {
    struct sockaddr local;
    socklen_t len = static_cast<socklen_t>( sizeof( local ) );
    if( getsockname ( m_socket, &local, &len ) < 0 )
      return -1;
    else
      return ntohs( (reinterpret_cast<struct sockaddr_in*>( &local ) )->sin_port );
  }

  const std::string ConnectionTCPBase::localInterface() const
  {
    struct sockaddr_storage local;
    socklen_t len = static_cast<socklen_t>( sizeof( local ) );
    if( getsockname( m_socket, reinterpret_cast<struct sockaddr*>( &local ), &len ) < 0 )
      return EmptyString;
    else
    {
      char buffer[INET6_ADDRSTRLEN];
      int err = getnameinfo( reinterpret_cast<struct sockaddr*>( &local ), len, buffer, sizeof( buffer ),
                             0, 0, NI_NUMERICHOST );
      if( !err )
        return buffer;
      else
        return EmptyString;
    }
  }

}
