/**
* Copyright (c) 2022 船山科技 chuanshantech.com
OkEDU-Classroom is licensed under Mulan PubL v2.
You can use this software according to the terms and conditions of the Mulan
PubL v2. You may obtain a copy of Mulan PubL v2 at:
        http://license.coscl.org.cn/MulanPubL-2.0
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PubL v2 for more details.
*/

#include "conference.h"
#include "disco.h"

namespace gloox {
    Conference::Conference() : StanzaExtension(ExtConference) {
      m_valid = true;
    }

    Conference::Conference(const Tag *tag) : StanzaExtension(ExtConference) {
      if (!tag || tag->name() != "x" || tag->xmlns() != XMLNS_X_CONFERENCE)
        return;
      m_jid = JID(tag->findAttribute("jid"));
      m_valid = true;
    }

    Tag *Conference::tag() const {
      if (!m_valid)
        return nullptr;

      Tag *x = new Tag("x", XMLNS, XMLNS_X_CONFERENCE);
      x->addAttribute("jid", m_jid.full());
    }

    const std::string &Conference::filterString() const {
      static const std::string filter =
          "/message/x[@xmlns='" + XMLNS_X_CONFERENCE + "']";
      return filter;
    }

} // namespace gloox
