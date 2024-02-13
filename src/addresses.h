/*
  Copyright (c) 2006-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef ADDRESSES_H__
#define ADDRESSES_H__

#include "gloox.h"
#include "jid.h"
#include "stanzaextension.h"

#include <string>

namespace gloox {

    class Tag;

    /**
     * @brief XEP-0033: Extended Stanza Addressing
     *
     * XEP version: 1.0
     * @author GaoJie <cto@chuanshaninfo.com>
     * @since 1.0
     */
    class GLOOX_API Addresses : public StanzaExtension {

    public:
//      enum Type {
//        to,cc,bcc,replyto,replyroom,noreply,ofrom
//      };

        struct Address {
            std::string type;
            JID jid;
            std::string uri;
            std::string node;
            std::string desc;
            std::string delivered;
        };

        /**
         * Constructs a new object and fills it according to the parameters.
         * @param from The JID of the original sender or the entity that delayed the sending.
         * @param stamp The datetime stamp of the original send.
         * @param reason An optional natural language reason for the delay.
         */
        Addresses(const std::list<Address> &addresses);

        /**
         * Constructs a new object from the given Tag.
         * @param tag The Tag to parse.
         */
        Addresses(const Tag *tag = 0);

        /**
         * Virtual Destructor.
         */
        virtual ~Addresses();


        const std::list<Address> &addresses() const { return m_addressList; }


        // reimplemented from StanzaExtension
        virtual const std::string &filterString() const;

        // reimplemented from StanzaExtension
        virtual StanzaExtension *newInstance(const Tag *tag) const {
          return new Addresses(tag);
        }

        // reimplemented from StanzaExtension
        virtual Tag *tag() const;

        // reimplemented from StanzaExtension
        virtual StanzaExtension *clone() const {
          return new Addresses(*this);
        }

    private:
        std::list<Address> m_addressList;
    };

}

#endif
