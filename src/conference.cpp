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
        if (!(tag && tag->name() == "conference" && tag->xmlns() == XMLNS_JITSI_FOCUS))
            return;

        /**
         *  <conference machine-uid=\"cc1bc7f4469a0a0071ab2ec72a7667bc\"
                room=\"test@conference.meet.chuanshaninfo.com\"
                xmlns=\"http://jitsi.org/protocol/focus\">
                <property name=\"startAudioMuted\" value=\"9\"/>
                <property name=\"startVideoMuted\" value=\"9\"/>
                <property name=\"rtcstatsEnabled\" value=\"false\"/>
                </conference>
         */
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

    Tag *Conference::tag() const {
        if (!m_valid)
            return nullptr;

        Tag *x = new Tag("conference", XMLNS, XMLNS_JITSI_FOCUS);
        x->addAttribute("room", m_jid.full());
        x->addAttribute("machine-uid", uid);

        //properties
        std::map<std::string , std::string>::const_iterator it = properties.begin();
        for (; it != properties.end(); ++it) {
            Tag *t = new Tag("property");
            t->addAttribute("name", it->first);
            t->addAttribute("value", it->second);
            x->addChild(t);
        }

        return x;
    }

    const std::string &Conference::filterString() const {
        static const std::string filter =
                "/conference[@xmlns='" + XMLNS_JITSI_FOCUS + "']";
        return filter;
    }

} // namespace gloox
