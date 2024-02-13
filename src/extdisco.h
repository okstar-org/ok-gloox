/*
  Copyright (c) 2007-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL )
#ifndef EXT_DISCO_H__
#define EXT_DISCO_H__

#include "gloox.h"
#include "stanzaextension.h"

#include <string>

namespace gloox
{

  class Tag;

  /**
   * @brief XEP-0215: External Service Discovery
   *
   * XEP version: 1.0
   * @author GaoJie <cto@chuanshaninfo.com>
   * @since 1.0
   */
  class GLOOX_API ExtDisco : public StanzaExtension
  {
    public:

      /**
       *  <service host='stun.shakespeare.lit'
             port='9998'
             transport='udp'
             type='stun'
             password='guest'
             username='guest'
             />
       */
      typedef struct {
          std::string host;
          int port;
          std::string type; //stun,turn
          std::string  transport; //udp,tcp
          std::string  username;
          std::string  password;
      }  Service;

      /**
       * Constructs a new object from the given Tag.
       * @param tag A Tag to parse.
       */
      ExtDisco( const Tag* tag = nullptr);

      /**
       * Virtual destructor.
       */
      virtual ~ExtDisco() {}

      /**
       * Returns the extension's saved nickname.
       * @return The nickname.
       */
      const std::list<Service> services() const { return m_services; }

      // reimplemented from StanzaExtension
      virtual const std::string& filterString() const;

      // reimplemented from StanzaExtension
      virtual StanzaExtension* newInstance( const Tag* tag ) const
      {
        return new ExtDisco( tag );
      }

      // reimplemented from StanzaExtension
      Tag* tag() const;



      // reimplemented from StanzaExtension
      virtual StanzaExtension* clone() const
      {
        return new ExtDisco( *this );
      }

      static Tag* newRequest() ;

    private:
      std::list<Service> m_services;
  };

}

#endif // EXT_DISCO_H__

#endif // GLOOX_MINIMAL
