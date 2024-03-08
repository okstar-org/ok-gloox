/*
 *  Copyright (c) 2023 by Jakob Schröter <js@camaya.net>
 *  This file is part of the gloox library. http://camaya.net/gloox
 *
 *  This software is distributed under a license. The full license
 *  agreement can be found in the file LICENSE in this distribution.
 *  This software may not be copied, modified, sold or distributed
 *  other than expressed in the named license agreement.
 *
 *  This software is distributed without any warranty.
 */


#include "../util.h"
#include "../gloox.h"

#include <cstdio>
#include <ctime>

using namespace gloox;

namespace helper
{
    static const char* protocolNames[] =
        { "TLS 1.0", "TLS 1.1", "TLS 1.2", "TLS 1.3", "DTLS 1.0", "DTLS 1.2", "Unknown protocol" };

    void printfCert( const CertInfo &certinfo )
    {
      time_t from( certinfo.date_from );
      time_t to( certinfo.date_to );

      printf( "status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n",
          certinfo.status, certinfo.issuer.c_str(), certinfo.server.c_str(),
          util::lookup( certinfo.protocol, protocolNames ).c_str(), certinfo.mac.c_str(), certinfo.cipher.c_str(),
          certinfo.compression.c_str() );

      printf( "from: %s", ctime( &from ) );
      printf( "to:   %s", ctime( &to ) );
    }

}
