/*
 *  Copyright (c) 2018-2023 by Jakob Schröter <js@camaya.net>
 *  This file is part of the gloox library. http://camaya.net/gloox
 *
 *  This software is distributed under a license. The full license
 *  agreement can be found in the file LICENSE in this distribution.
 *  This software may not be copied, modified, sold or distributed
 *  other than expressed in the named license agreement.
 *
 *  This software is distributed without any warranty.
 */


#if !defined( GLOOX_MINIMAL ) || defined( WANT_REFERENCES )

#include "reference.h"
#include "tag.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

namespace gloox
{

  /* chat marker type values */
  static const char* referenceValues [] = {
    "mention",
    "data"
  };

  static inline Reference::ReferenceType referenceType( const std::string& name )
  {
    return static_cast<Reference::ReferenceType>( util::lookup( name, referenceValues ) );
  }

  Reference::Reference( const Tag* tag )
    : StanzaExtension( ExtReference ),
      m_type( InvalidType ), m_begin( 0 ), m_end( 0 )
  {
    if( !tag || tag->name() != "reference" || tag->xmlns() != XMLNS_REFERENCES )
      return;

    m_type = referenceType( tag->findAttribute("type" ) );
    m_begin = atoi( tag->findAttribute( "begin" ).c_str() );
    m_end = atoi( tag->findAttribute( "end" ).c_str() );
    m_anchor = tag->findAttribute( "anchor" );
    m_uri = tag->findAttribute( "uri" );
  }

  const std::string& Reference::filterString() const
  {
    static const std::string filter = "/message/reference[@xmlns='" + XMLNS_REFERENCES + "']";
    return filter;
  }

  Tag* Reference::tag() const
  {
    if( m_type == InvalidType )
      return 0;

    Tag* t = new Tag( "reference" );
    t->setXmlns( XMLNS_REFERENCES );

    t->addAttribute( "type", util::lookup( m_type, referenceValues ) );
    t->addAttribute( "uri", m_uri );

    if( m_begin != m_end )
    {
      t->addAttribute( "begin", m_begin );
      t->addAttribute( "end", m_end );
    }

    switch( m_type )
    {
      case Mention:
        break;
      case Data:
        if( !m_anchor.empty() )
          t->addAttribute( "anchor", m_anchor );
        break;
      default:
        break;
    }

    return t;
  }

}

#endif // GLOOX_MINIMAL
