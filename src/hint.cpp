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


#if !defined( GLOOX_MINIMAL ) || defined( WANT_HINTS )

#include "hint.h"
#include "tag.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

namespace gloox
{

  /* chat marker type values */
  static const char* hintValues [] = {
    "no-permanent-store",
    "no-store",
    "no-copy",
    "store"
  };

  static inline Hint::HintType hintType( const std::string& name )
  {
    return static_cast<Hint::HintType>( util::lookup( name, hintValues ) );
  }

  Hint::Hint( const Tag* tag )
    : StanzaExtension( ExtHint ),
      m_type( InvalidType )
  {
    if( !tag || tag->xmlns() != XMLNS_HINTS )
      return;

    m_type = hintType( tag->name() );
  }

  const std::string& Hint::filterString() const
  {
    static const std::string filter = "/message/*[@xmlns='" + XMLNS_HINTS + "']";
    return filter;
  }

  Tag* Hint::tag() const
  {
    if( m_type == InvalidType )
      return 0;

    Tag* t = new Tag( util::lookup( m_type, hintValues ) );
    t->setXmlns( XMLNS_HINTS );

    return t;
  }

}

#endif // GLOOX_MINIMAL
