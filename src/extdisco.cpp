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

#include "extdisco.h"
#include "stanzaextension.h"
#include "tag.h"

namespace gloox {

    ExtDisco::ExtDisco(const Tag *tag)
        : StanzaExtension(ExtSrvDisco) {
      if (!tag) {
        m_valid = false;
        return;
      }

      const Tag *tags = tag->findTag(filterString());
      TagList srvs = tags->findChildren("service");

      TagList::const_iterator it = srvs.begin();
      for (; it != srvs.end(); ++it) {
        Service srv;
        srv.host = (*it)->findAttribute("host");
        srv.port = std::atoi((*it)->findAttribute("port").data());
        srv.type = (*it)->findAttribute("type");
        srv.transport = (*it)->findAttribute("transport");
        srv.username = (*it)->findAttribute("username");
        srv.password = (*it)->findAttribute("password");
        m_services.push_back(srv);
      }
      m_valid = true;
    }

    const std::string &ExtDisco::filterString() const {
      static const std::string filter =
          "/iq/services[@xmlns='" + XMLNS_EXTERNAL_SERVICE_DISCOVERY + "']";
      return filter;
    }

    Tag *ExtDisco::tag() const {
      if (m_services.empty())
        return 0;

      Tag *n = new Tag("services", XMLNS, XMLNS_EXTERNAL_SERVICE_DISCOVERY);
      std::list<Service>::const_iterator it = m_services.begin();
      for (; it != m_services.end(); ++it) {
        Tag *srv = new Tag(n, "service");
        srv->addAttribute("host", (*it).host);
        srv->addAttribute("port", (*it).port);
        srv->addAttribute("type", (*it).type);
        srv->addAttribute("transport", (*it).transport);
        srv->addAttribute("username", (*it).username);
        srv->addAttribute("password", (*it).password);
      }
      return n;
    }

    Tag *ExtDisco::newRequest() {
      Tag *n = new Tag("services", XMLNS, XMLNS_EXTERNAL_SERVICE_DISCOVERY);
      return n;
    }

}

#endif // GLOOX_MINIMAL
