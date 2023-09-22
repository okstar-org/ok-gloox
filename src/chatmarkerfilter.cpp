/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL ) || defined( WANT_CHATMARKER )

#include "chatmarkerfilter.h"
#include "chatmarkerhandler.h"
#include "messageeventhandler.h"
#include "messagesession.h"
#include "message.h"
#include "chatmarker.h"

namespace gloox
{

  ChatMarkerFilter::ChatMarkerFilter( MessageSession* parent )
    : MessageFilter( parent ), m_chatMarkerHandler( 0 ), m_lastSent( ChatMarkerInvalid ),
      m_enableChatMarkers( true )
  {
  }

  ChatMarkerFilter::~ChatMarkerFilter()
  {
  }

  void ChatMarkerFilter::filter( Message& msg )
  {
    if( m_enableChatMarkers && m_chatMarkerHandler )
    {
      const ChatMarker* marker = msg.findExtension<ChatMarker>( ExtChatMarkers );

      if( !marker )
        return;
      m_enableChatMarkers = ( marker->marker() != ChatMarkerInvalid );

      if( m_enableChatMarkers && msg.body().empty() )
        m_chatMarkerHandler->handleChatMarker( msg.from(), marker->marker(), gloox::EmptyString );
    }
  }

  void ChatMarkerFilter::setMessageMarkable()
  {
    Message m( Message::Chat, m_parent->target() );
    m.addExtension( new ChatMarker( ChatMarkerInvalid ) );

//     m_lastSent = marker;

    send( m );
  }

  void ChatMarkerFilter::decorate( Message& msg )
  {
    if( m_enableChatMarkers )
      msg.addExtension( new ChatMarker( ChatMarkerInvalid ) );
  }

}

#endif // GLOOX_MINIMAL
