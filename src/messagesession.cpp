/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL ) || defined( WANT_MESSAGESESSION )

#include "messagesession.h"
#include "messagefilter.h"
#include "messagehandler.h"
#include "clientbase.h"
#include "disco.h"
#include "message.h"
#include "util.h"

namespace gloox
{

  MessageSession::MessageSession( ClientBase* parent, const JID& jid, bool wantResourceTracking, int types, bool honorTID )
    : m_parent( parent ), m_target( jid ), m_messageHandler( 0 ),
      m_types( types ), m_wantResourceTracking( wantResourceTracking ), m_hadMessages( false ), m_honorThreadID( honorTID )
  {
    if( m_parent )
      m_parent->registerMessageSession( this );
  }

  MessageSession::~MessageSession()
  {
    util::clearList( m_messageFilterList );
  }

  void MessageSession::handleMessage( Message& msg )
  {
    if( m_wantResourceTracking && msg.from().resource() != m_target.resource() )
      setResource( msg.from().resource() );

    if( !m_hadMessages )
    {
      m_hadMessages = true;
      if( msg.thread().empty() )
      {
        m_thread = "gloox" + m_parent->getID();
        msg.setThread( m_thread );
      }
      else
        m_thread = msg.thread();
    }

    MessageFilterList::const_iterator it = m_messageFilterList.begin();
    for( ; it != m_messageFilterList.end(); ++it )
      (*it)->filter( msg );

    if( m_messageHandler )
      m_messageHandler->handleMessage( msg, this );
  }

  std::string MessageSession::send( const std::string& message )
  {
    return send( message, EmptyString );
  }

  std::string MessageSession::send( const std::string& message, const std::string& subject, const StanzaExtensionList& sel )
  {
    if( !m_hadMessages )
    {
      m_thread = "gloox" + m_parent->getID();
      m_hadMessages = true;
    }

    Message m( Message::Chat, m_target.full(), message, subject, m_thread );
    m.setID( m_parent->getID() );
    decorate( m );

    if( sel.size() )
    {
      StanzaExtensionList::const_iterator it = sel.begin();
      for( ; it != sel.end(); ++it )
        m.addExtension( (*it));
    }

    m_parent->send( m );
    return m.id();
  }

  void MessageSession::send( const Message& msg )
  {
    m_parent->send( msg );
  }

  void MessageSession::decorate( Message& msg )
  {
    util::ForEach( m_messageFilterList, &MessageFilter::decorate, msg );
  }

  void MessageSession::resetResource()
  {
    m_target.setResource( EmptyString );
  }

  void MessageSession::setResource( const std::string& resource )
  {
    m_target.setResource( resource );
  }

  void MessageSession::disposeMessageFilter( MessageFilter* mf )
  {
    removeMessageFilter( mf );
    delete mf;
  }

}

#endif // GLOOX_MINIMAL
