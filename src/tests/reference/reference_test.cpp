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
#include "../../reference.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]

int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  Reference *i = 0;
  Tag* t = 0;


  // -------
  name = "parsing 0 tag";
  i = new Reference( 0 );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing unpopulated tag";
  Tag* r = new Tag( "reference" );
  r->setXmlns( XMLNS_REFERENCES );
  i = new Reference( r );
  if( i->tag() != 0 )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;

  // -------
  name = "parsing mention type";
  r->addAttribute( "type", "mention" );
  r->addAttribute( "uri", "someuri" );
  r->addAttribute( "begin", 12 );
  r->addAttribute( "end", 19 );
  i = new Reference( r );
  t = i->tag();
  if( !t || r->xml() != t->xml() )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n%s\n", name.c_str(), t->xml().c_str() );
  }
  delete t;
  t = 0;
  delete i;
  i = 0;


  // -------
  name = "parsing data type";
  r->addAttribute( "type", "data" );
  r->addAttribute( "uri", "someuri" );
  r->addAttribute( "begin", 12 );
  r->addAttribute( "end", 19 );
  r->addAttribute( "anchor", "someanchor" );
  i = new Reference( r );
  t = i->tag();
  if( !t || r->xml() != t->xml() )
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
  i = new Reference( 0 );
  Reference* j = i->clone();
  if( !j )
  {
    ++fail;
    fprintf( stderr, "test '%s' failed\n", name.c_str() );
  }
  delete i;
  i = 0;
  delete j;
  j = 0;


  delete r;




  if( fail == 0 )
  {
    printf( "Reference: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Reference: %d test(s) failed\n", fail );
    return 1;
  }

}
