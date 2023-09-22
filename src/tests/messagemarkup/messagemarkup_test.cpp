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

#include "../../tag.h"
#include "../../messagemarkup.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  MessageMarkup *i = 0;
  Tag* t = 0;


  // -------
  name = "parsing 0 tag";
  i = new MessageMarkup( 0 );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing unpopulated tag";
  Tag* m = new Tag( "markup" );
  m->setXmlns( XMLNS_MESSAGE_MARKUP );
  i = new MessageMarkup( m );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing span tag";
  Tag* s = new Tag( m, "span" );
  s->addAttribute( "start", 12 );
  s->addAttribute( "end", 19 );
  new Tag( s, "emphasis" );
  new Tag( s, "code" );
  new Tag( s, "deleted" );
  i = new MessageMarkup( m );
  t = i->tag();
  if( !t || t->name() != "markup" || t->xmlns() != XMLNS_MESSAGE_MARKUP
    || !t->hasChild( "span" ) || !t->findChild( "span" )->hasAttribute( "start", "12" )
    || !t->findChild( "span" )->hasAttribute( "end", "19" )
    || m->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete t;
  t = 0;
  delete i;
  i = 0;


  // -------
  name = "parsing bcode tag";
  m->removeChild( s );
  delete s;
  s = new Tag( m, "bcode" );
  s->addAttribute( "start", 1 );
  s->addAttribute( "end", 12 );
  i = new MessageMarkup( m );
  t = i->tag();
  if( !t || t->name() != "markup" || t->xmlns() != XMLNS_MESSAGE_MARKUP
    || !t->hasChild( "bcode" ) || !t->findChild( "bcode" )->hasAttribute( "start", "1" )
    || !t->findChild( "bcode" )->hasAttribute( "end", "12" )
    || m->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete t;
  t = 0;
  delete i;
  i = 0;

  // -------
  name = "parsing bquote tag";
  m->removeChild( s );
  delete s;
  s = new Tag( m, "bquote" );
  s->addAttribute( "start", 8 );
  s->addAttribute( "end", 17 );
  i = new MessageMarkup( m );
  t = i->tag();
  if( !t || t->name() != "markup" || t->xmlns() != XMLNS_MESSAGE_MARKUP
    || !t->hasChild( "bquote" ) || !t->findChild( "bquote" )->hasAttribute( "start", "8" )
    || !t->findChild( "bquote" )->hasAttribute( "end", "17" )
    || m->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete t;
  t = 0;
  delete i;
  i = 0;

  // -------
  name = "parsing list tag";
  m->removeChild( s );
  delete s;
  s = new Tag( m, "list" );
  s->addAttribute( "start", 4 );
  s->addAttribute( "end", 9 );
  Tag* l = new Tag( s, "li" );
  l->addAttribute( "start", 24 );
  l = new Tag( s, "li" );
  l->addAttribute( "start", 36 );
  l = new Tag( s, "li" );
  l->addAttribute( "start", 48 );
  i = new MessageMarkup( m );
  t = i->tag();
  if( !t || t->name() != "markup" || t->xmlns() != XMLNS_MESSAGE_MARKUP
    || !t->hasChild( "list" ) || !t->findChild( "list" )->hasAttribute( "start", "4" )
    || !t->findChild( "list" )->hasAttribute( "end", "9" )
    || m->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete t;
  t = 0;
  delete i;
  i = 0;


  // -------
  name = "cloning";
  i = new MessageMarkup( 0 );
  MessageMarkup* j = i->clone();
  if( !j )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;
  delete j;
  j = 0;


  delete m;




  if( fail == 0 )
  {
    printf( "MessageMarkup: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "MessageMarkup: %d test(s) failed\n", fail );
    return 1;
  }

}
