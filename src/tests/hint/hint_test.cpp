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
#include "../../hint.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Hint *i = 0;
  Tag* t = 0;


  // -------
  name = "parsing 0 tag";
  i = new Hint( 0 );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing no-permanent-store tag";
  Tag* h = new Tag( "no-permanent-store" );
  h->setXmlns( XMLNS_HINTS );
  i = new Hint( h );
  t = i->tag();
  if( !t || h->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;
  delete h;
  h = 0;

  // -------
  name = "parsing no-store type";
  h = new Tag( "no-store" );
  h->setXmlns( XMLNS_HINTS );
  i = new Hint( h );
  t = i->tag();
  if( !t || h->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;
  delete h;
  h = 0;

  // -------
  name = "parsing no-copy type";
  h = new Tag( "no-copy" );
  h->setXmlns( XMLNS_HINTS );
  i = new Hint( h );
  t = i->tag();
  if( !t || h->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;
  delete h;
  h = 0;

  // -------
  name = "parsing store type";
  h = new Tag( "store" );
  h->setXmlns( XMLNS_HINTS );
  i = new Hint( h );
  t = i->tag();
  if( !t || h->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete i;
  i = 0;
  delete t;
  t = 0;
  delete h;
  h = 0;


  // -------
  name = "cloning";
  i = new Hint( 0 );
  Hint* j = i->clone();
  if( !j )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;
  delete j;
  j = 0;





  if( fail == 0 )
  {
    printf( "Hint: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Hint: %d test(s) failed\n", fail );
    return 1;
  }

}
