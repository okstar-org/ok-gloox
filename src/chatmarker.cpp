/*
  Copyright (c) 2007-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL ) || defined( WANT_CHATMARKER )

#include "chatmarker.h"
#include "tag.h"
#include "util.h"

namespace gloox
{

  /* chat marker type values */
  static const char* markerValues [] = {
    "received",
    "displayed",
    "acknowledged"
  };

  static inline ChatMarkerType chatMarkerType( const std::string& type )
  {
    return static_cast<ChatMarkerType>( util::lookup2( type, markerValues ) );
  }

  ChatMarker::ChatMarker( const Tag* tag )
    : StanzaExtension( ExtChatMarkers ),
      m_marker( ChatMarkerInvalid )
  {
    if( tag )
      m_marker = chatMarkerType( tag->name() );
  }

  const std::string& ChatMarker::filterString() const
  {
    static const std::string filter =
           "/message/received[@xmlns='" + XMLNS_CHAT_MARKERS + "']"
           "|/message/displayed[@xmlns='" + XMLNS_CHAT_MARKERS + "']"
           "|/message/acknowledged[@xmlns='" + XMLNS_CHAT_MARKERS + "']";
    return filter;
  }

  Tag* ChatMarker::tag() const
  {
    if( m_marker == ChatMarkerInvalid )
      return 0;

    return new Tag( util::lookup2( m_marker, markerValues ), XMLNS, XMLNS_CHAT_MARKERS );
  }

}

#endif // GLOOX_MINIMAL
