/*
 *  Copyright (c) 2004-2023 by Jakob Schröter <js@camaya.net>
 *  This file is part of the gloox library. http://camaya.net/gloox
 *
 *  This software is distributed under a license. The full license
 *  agreement can be found in the file LICENSE in this distribution.
 *  This software may not be copied, modified, sold or distributed
 *  other than expressed in the named license agreement.
 *
 *  This software is distributed without any warranty.
 */

#include "helper.h"

#include "../client.h"
#include "../messagesessionhandler.h"
#include "../messageeventhandler.h"
#include "../messageeventfilter.h"
#include "../chatstatehandler.h"
#include "../chatstatefilter.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../message.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../connectiontcpclient.h"
#include "../connectionsocks5proxy.h"
#include "../connectionhttpproxy.h"
#include "../connectiontcpclient.h"
#include "../connectionwebsocket.h"
#include "../connectiontls.h"
#include "../messagehandler.h"
#include "../messageevent.h"
using namespace gloox;

#ifndef _WIN32
# include <unistd.h>
#endif

#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class MessageTest : public MessageSessionHandler, ConnectionListener, LogHandler,
                    MessageEventHandler, MessageHandler, ChatStateHandler
{
  public:
    MessageTest() : m_session( 0 ), m_messageEventFilter( 0 ), m_chatStateFilter( 0 ) {}

    virtual ~MessageTest() {}

    void start()
    {

      JID jid( "admin@jabber.cc/gloox" );
      j = new Client( jid, "y4crwcw5t5wt5" );
//       j->setServer ("127.0.0.1");
//       j->setPort( 6222 );
//       j->setTls( TLSDisabled );
      j->registerConnectionListener( this );
      j->registerMessageSessionHandler( this, 0 );
      j->disco()->setIdentity( "client", "bot" );
      j->disco()->addFeature( XMLNS_CHAT_STATES );

      j->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );

// this code connects to a server using XMPP over WebSocket
//       ConnectionWebSocket* conn = new ConnectionWebSocket( j,
//                                       new ConnectionTLS(
//                                           new ConnectionTCPClient( j->logInstance(),  "jabber.cc", 5281 ),
//                                           j->logInstance() ),
//                                       j->logInstance(), "jabber.cc" );
//       conn->setPath( "/xmpp-websocket" );
//       j->setConnectionImpl( conn );
//       j->setResource( "websocket" );

// this code connects to a jabber server through a SOCKS5 proxy
//
//       ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
//                                   new ConnectionTCPClient( j->logInstance(),
//                                                      "sockshost", 1080 ),
//                                   j->logInstance(), "example.net" );
//       conn->setProxyAuth( "socksuser", "sockspwd" );
//       j->setConnectionImpl( conn );

//
// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
//
//       ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
//       ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
//       conn1->setProxyAuth( "socksuser", "sockspwd" );
//       ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
//       conn2->setProxyAuth( "httpuser", "httppwd" );
//       j->setConnectionImpl( conn2 );


      if( j->connect( false ) )
      {
        ConnectionError ce = ConnNoError;
        while( ce == ConnNoError )
        {
          ce = j->recv();
        }
        printf( "ce: %d\n", ce );
      }

      delete( j );
    }

    virtual void onConnect()
    {
      printf( "connected!!!\n" );
    }

    virtual void onDisconnect( ConnectionError e )
    {
      printf( "message_test: disconnected: %d\n", e );
      if( e == ConnAuthenticationFailed )
        printf( "auth failed. reason: %d\n", j->authError() );
    }

    virtual bool onTLSConnect( const CertInfo& info )
    {
      helper::printfCert( info );
      return ( info.status == CertOk );
    }

    virtual void handleMessage( const Message& msg, MessageSession * /*session*/ )
    {
      printf( "type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
              msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str() );

      std::string re = "You said:\n> " + msg.body() + "\nI like that statement.";
      std::string sub;
      if( !msg.subject().empty() )
        sub = "Re: " +  msg.subject();

      m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
#if defined( WIN32 ) || defined( _WIN32 )
      Sleep( 1000 );
#else
      sleep( 1 );
#endif
      m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
      m_chatStateFilter->setChatState( ChatStateComposing );
#if defined( WIN32 ) || defined( _WIN32 )
      Sleep( 2000 );
#else
      sleep( 2 );
#endif
      m_session->send( re, sub );

      if( msg.body() == "quit" )
        j->disconnect();
    }

    virtual void handleMessageEvent( const JID& from, const MessageEvent* event )
    {
      if( event )
        printf( "received event: %d from: %s\n", event->event(), from.full().c_str() );
    }

    virtual void handleChatState( const JID& from, ChatStateType state )
    {
      printf( "received state: %d from: %s\n", state, from.full().c_str() );
    }

    virtual void handleMessageSession( MessageSession *session )
    {
      printf( "got new session\n");
      // this example can handle only one session. so we get rid of the old session
      j->disposeMessageSession( m_session );
      m_session = session;
      m_session->registerMessageHandler( this );
      m_messageEventFilter = new MessageEventFilter( m_session );
      m_messageEventFilter->registerMessageEventHandler( this );
      m_chatStateFilter = new ChatStateFilter( m_session );
      m_chatStateFilter->registerChatStateHandler( this );
    }

    virtual void handleLog( LogLevel level, LogArea area, const std::string& message )
    {
      printf("log: level: %d, area: %d, %s\n", level, area, message.c_str() );
    }

  private:
    Client *j;
    MessageSession *m_session;
    MessageEventFilter *m_messageEventFilter;
    ChatStateFilter *m_chatStateFilter;
};

int main( int /*argc*/, char** /*argv*/ )
{
  MessageTest *r = new MessageTest();
  r->start();
  delete( r );
  return 0;
}
