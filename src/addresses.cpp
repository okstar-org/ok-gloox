/*
  Copyright (c) 2006-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#include "addresses.h"
#include "tag.h"

namespace gloox {

    Addresses::Addresses(const std::list<Address> &addresses)
        : StanzaExtension(ExtAddresses), m_addressList(addresses) {

    }


    Addresses::Addresses(const Tag *tag)
        : StanzaExtension(ExtAddresses) {
      if (!tag || tag->xmlns() != XMLNS_ADDRESSES)
        return;

      TagList addressList = tag->findChildren("address");
      TagList::const_iterator it = addressList.begin();
      while (it != addressList.end()) {
        Address addr;
        addr.type = (*it)->findAttribute("type");
        addr.jid = JID((*it)->findAttribute("jid"));
        addr.uri=(*it)->findAttribute("uri");
        addr.node=(*it)->findAttribute("node");
        addr.desc=(*it)->findAttribute("desc");
        addr.delivered=(*it)->findAttribute("delivered");
        m_addressList.push_back(addr);
      }
      m_valid = true;
    }

    Addresses::~Addresses() {
    }

    const std::string &Addresses::filterString() const {
      static const std::string filter =
          "/message/addresses[@xmlns='" + XMLNS_ADDRESSES + "']";
      return filter;
    }

    Tag *Addresses::tag() const {
      if (!m_valid)
        return 0;

      Tag *t = new Tag("addresses");
      t->addAttribute(XMLNS, XMLNS_ADDRESSES);

      std::list<Address>::const_iterator it = m_addressList.begin();
      while (it != m_addressList.end()){
        Tag* a = new Tag(t, "address");
        a->addAttribute("type", (*it).type);
        a->addAttribute("jid", (*it).jid.full());
        a->addAttribute("uri", (*it).uri);
        a->addAttribute("node", (*it).node);
        a->addAttribute("desc", (*it).desc);
        a->addAttribute("delivered", (*it).delivered);
      }

      return t;
    }

}


