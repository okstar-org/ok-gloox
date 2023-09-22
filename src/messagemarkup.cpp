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


#if !defined( GLOOX_MINIMAL ) || defined( WANT_MESSAGEMARKUP )

#include "messagemarkup.h"
#include "tag.h"
#include "util.h"

#include <stdlib.h>
#include <stdio.h>

namespace gloox
{

  /* chat marker type values */
  static const char* markupValues [] = {
    "span",
    "bcode",
    "list",
    "bquote"
  };

  static inline MessageMarkup::MarkupType markupType( const std::string& name )
  {
    return static_cast<MessageMarkup::MarkupType>( util::lookup( name, markupValues ) );
  }

  MessageMarkup::MessageMarkup( const Tag* tag )
    : StanzaExtension( ExtMessageMarkup )
  {
    if( !tag || tag->name() != "markup" || tag->xmlns() != XMLNS_MESSAGE_MARKUP )
      return;

    m_lang = tag->findAttribute( "xml:lang" );

    const TagList& c = tag->children();
    TagList::const_iterator it = c.begin();
    for( ; it != c.end(); ++it )
    {
      MarkupType type = markupType( (*it)->name() );

      int start = 0;
      start = atoi( (*it)->findAttribute( "start" ).c_str() );
      int end = start;
      end = atoi( (*it)->findAttribute( "end" ).c_str() );
      int spans = 0;

      switch( type )
      {
        case Span:
        {
          if( (*it)->hasChild( "emphasis" ) )
            spans |= Emphasis;
          if( (*it)->hasChild( "code" ) )
            spans |= Code;
          if( (*it)->hasChild( "deleted" ) )
            spans |= Deleted;
          m_markups.push_back( Markup( Span, start, end, spans, IntList() ) );
          break;
        }
        case BCode:
        {
          m_markups.push_back( Markup( BCode, start, end, 0, IntList() ) );
          break;
        }
        case List:
        {
          IntList lis;
          const TagList& l = (*it)->children();
          TagList::const_iterator lit = l.begin();
          for( ; lit != l.end(); ++lit )
          {
            if( (*lit) && (*lit)->name() == "li" && (*lit)->hasAttribute( "start" ) )
              lis.push_back( atoi( (*lit)->findAttribute( "start" ).c_str() ) );
          }
          m_markups.push_back( Markup( List, start, end, 0, lis ) );
          break;
        }
        case BQuote:
        {
          m_markups.push_back( Markup( BQuote, start, end, 0, IntList() ) );
          break;
        }
        default:
        {
          // ignore to be compatible with future versions
          break;
        }
      }
    }
  }

  const std::string& MessageMarkup::filterString() const
  {
    static const std::string filter = "/message/markup[@xmlns='" + XMLNS_MESSAGE_MARKUP + "']";
    return filter;
  }

  Tag* MessageMarkup::tag() const
  {
    if( !m_markups.size() )
      return 0;

    Tag* t = new Tag( "markup" );
    t->setXmlns( XMLNS_MESSAGE_MARKUP );

    if( !m_lang.empty() )
      t->addAttribute( "xml:lang", m_lang );

    MarkupList::const_iterator it = m_markups.begin();
    for( ; it != m_markups.end(); ++it )
    {
      if( (*it).type == InvalidType )
        continue;

      Tag* m = new Tag( t, util::lookup( (*it).type, markupValues ) );
      m->addAttribute( "start", (*it).start );
      m->addAttribute( "end", (*it).end );

      if( (*it).type == Span )
      {
        if( ( (*it).spans & Emphasis ) == Emphasis )
          new Tag( m, "emphasis" );
        if( ( (*it).spans & Code ) == Code )
          new Tag( m, "code" );
        if( ( (*it).spans & Deleted ) == Deleted )
          new Tag( m, "deleted" );
      }
      else if( ( (*it).type == List ) && (*it).lis.size() )
      {
        IntList::const_iterator lit = (*it).lis.begin();
        for( ; lit != (*it).lis.end(); ++lit )
          new Tag( m, "li", "start", util::int2string( (*lit) ) );
      }
    }

    return t;
  }

}

#endif // GLOOX_MINIMAL
