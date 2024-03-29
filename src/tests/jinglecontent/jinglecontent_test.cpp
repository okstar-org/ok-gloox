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

#define JINGLECONTENT_TEST
#include "../../gloox.h"
#include "../../jid.h"
#include "../../tag.h"
#include "../../jinglecontent.h"
using namespace gloox;

#include <stdio.h>
#include <locale.h>
#include <string>
#include <cstdio> // [s]print[f]


int main( int /*argc*/, char** /*argv*/ )
{
  int fail = 0;
  std::string name;
  JID jid( "foo@bar/barfoo" );

  // -------
  {
    name = "invalid Jingle 1";
    Jingle::Content jc;
    Tag* t = jc.tag();
    if( t )
    {
      ++fail;
      fprintf( stderr, "test '%s' failed\n", name.c_str() );
    }
    delete t;
  }





  // -------
//   name = "Jingle::Session::Jingle/SEFactory test";
//   StanzaExtensionFactory sef;
//   sef.registerExtension( new Jingle::Session::Jingle() );
//   Tag* f = new Tag( "iq" );
//   new Tag( f, "jingle", "xmlns", XMLNS_JINGLE );
//   IQ iq( IQ::Get, JID() );
//   sef.addExtensions( iq, f );
//   const Jingle::Session::Jingle* se = iq.findExtension<Jingle::Session::Jingle>( ExtJingle );
//   if( se == 0 )
//   {
//     ++fail;
//     fprintf( stderr, "test '%s' failed\n", name.c_str() );
//   }
//   delete f;



  if( fail == 0 )
  {
    printf( "Jingle::Content: OK\n" );
    return 0;
  }
  else
  {
    fprintf( stderr, "Jingle::Content: %d test(s) failed\n", fail );
    return 1;
  }

}
