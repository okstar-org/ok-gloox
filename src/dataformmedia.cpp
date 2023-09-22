/*
 *  Copyright (c) 2019-2023 by Jakob Schröter <js@camaya.net>
 *  This file is part of the gloox library. http://camaya.net/gloox
 *
 *  This software is distributed under a license. The full license
 *  agreement can be found in the file LICENSE in this distribution.
 *  This software may not be copied, modified, sold or distributed
 *  other than expressed in the named license agreement.
 *
 *  This software is distributed without any warranty.
 */


#if !defined( GLOOX_MINIMAL ) || defined( WANT_DATAFORM ) || defined( WANT_ADHOC )

#include "dataformmedia.h"
#include "tag.h"

#include <stdlib.h>

namespace gloox
{

  DataFormMedia::DataFormMedia( const std::string& uri, const std::string& type, int height, int width )
    : m_uri( uri ), m_type( type ), m_height( height ), m_width( width )
  {
  }

  DataFormMedia::DataFormMedia( const Tag* tag )
    : m_height( 0 ), m_width( 0 )
  {
    if( !tag || tag->name() != "media" || tag->xmlns() != XMLNS_DATAFORM_MEDIA )
      return;

    m_height = atoi( tag->findAttribute( "height" ).c_str() );
    m_width = atoi( tag->findAttribute( "width" ).c_str() );

    Tag* uri = tag->findChild( "uri" );
    if( uri )
    {
      m_type = uri->findAttribute( "type" );
      m_uri = uri->cdata();
    }
  }

  Tag* DataFormMedia::tag() const
  {
    if( m_type.empty() || m_uri.empty() )
      return 0;

    Tag* t = new Tag( "media" );
    t->setXmlns( XMLNS_DATAFORM_MEDIA );

    if( m_height )
      t->addAttribute( "height", m_height );

    if( m_width )
      t->addAttribute( "width", m_width );

    Tag* uri = new Tag(t, "uri", m_uri );
    uri->addAttribute( "type", m_type );

    return t;
  }

}

#endif // GLOOX_MINIMAL
