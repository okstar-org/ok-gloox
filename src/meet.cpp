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

#include "meet.h"
#include "disco.h"

namespace gloox {

    Meet::Meet() : StanzaExtension(ExtMeet) {
        m_valid = true;
    }

    Meet::Meet(const gloox::JID &m_jid_, const std::string &uid_,
                           const std::map<std::string, std::string> &properties_)
            : StanzaExtension(ExtMeet), m_jid(m_jid_), uid(uid_) {
        properties = properties_;
        m_valid = true;
    }

    Meet::Meet(const Tag *tag) : StanzaExtension(ExtMeet) {
        if (!(tag && tag->name() == "conference" && tag->xmlns() == XMLNS_JITSI_FOCUS))
            return;

        m_jid = JID(tag->findAttribute("room"));
        uid = tag->findAttribute("machine-uid");
        TagList props = tag->findChildren("property");
        TagList::const_iterator it = props.begin();
        for (; it != props.end(); ++it) {
            auto name = (*it)->findAttribute("name");
            if (!name.empty()) {
                properties.insert(std::pair(name, (*it)->findAttribute("value")));
            }
        }

        m_valid = true;
    }

    Tag *Meet::tag() const {
        if (!m_valid)
            return nullptr;

        Tag *x = new Tag("conference", XMLNS, XMLNS_JITSI_FOCUS);
        x->addAttribute("room", m_jid.full());
        x->addAttribute("machine-uid", uid);

        //properties
        std::map<std::string, std::string>::const_iterator it = properties.begin();
        for (; it != properties.end(); ++it) {
            Tag *t = new Tag("property");
            t->addAttribute("name", it->first);
            t->addAttribute("value", it->second);
            x->addChild(t);
        }

        return x;
    }

    const std::string &Meet::filterString() const {
        static const std::string filter =
                "/conference[@xmlns='" + XMLNS_JITSI_FOCUS + "']";
        return filter;
    }

} // namespace gloox
