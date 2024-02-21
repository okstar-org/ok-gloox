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

#include "jitsiconference.h"
#include "disco.h"

namespace gloox {
    Conference::Conference() : StanzaExtension(ExtConference) {}

    Conference::Conference(const JID &focus, const std::string &room,
                           const std::string &machineUid, const bool disableRtx,
                           const bool enableLipSync, const bool openSctp)
            : StanzaExtension(ExtConference), m_focus(focus), m_room(room),
              m_machineUid(machineUid), m_disableRtx(disableRtx),
              m_enableLipSync(enableLipSync), m_openSctp(openSctp) {}

    Conference::Conference(const Tag *tag) : StanzaExtension(ExtConference) {
        if (!tag || tag->name() != "conference" || tag->xmlns() != JITSI_CONFERENCE)
            return;

        m_room = tag->findAttribute("room");
        m_machineUid = tag->findAttribute("machine-uid");

        const TagList &l = tag->findChildren("property");
        for (TagList::const_iterator it = l.begin(); it != l.end(); ++it) {
            const std::string &name = (*it)->findAttribute("name");
            const std::string &value = (*it)->findAttribute("value");
            if (name == "disableRtx") {
                m_disableRtx = value == "true";
            } else if (name == "enableLipSync") {
                m_enableLipSync = value == "true";
            } else if (name == "openSctp") {
                m_openSctp = value == "true";
            }
            if (name == "authentication") {
                m_auth = value == "true";
            }
        }
        m_ready = tag->findAttribute("ready") == "true";
        m_focusJid = JID(tag->findAttribute("focusjid"));
    }

    Conference::Conference(const Conference &conference)
            : StanzaExtension(ExtConference), m_focus(conference.m_focus),
              m_room(conference.m_room), m_machineUid(conference.m_machineUid),
              m_disableRtx(conference.m_disableRtx),
              m_enableLipSync(conference.m_enableLipSync),
              m_openSctp(conference.m_openSctp) {}

    Tag *Conference::tag() const {
        Tag *t = new Tag("conference", XMLNS, JITSI_CONFERENCE);
        t->addAttribute("room", m_room);
        t->addAttribute("machine-uid", m_machineUid);

        Tag *disableRtx = new Tag(t, "property");
        disableRtx->addAttribute("name", "disableRtx");
        disableRtx->addAttribute("value", m_disableRtx ? "true" : "false");

        Tag *enableLipSync = new Tag(t, "property");
        enableLipSync->addAttribute("name", "enableLipSync");
        enableLipSync->addAttribute("value", m_enableLipSync ? "true" : "false");

        Tag *openSctp = new Tag(t, "property");
        openSctp->addAttribute("name", "openSctp");
        openSctp->addAttribute("value", m_openSctp ? "true" : "false");


        return t;
    }

    const std::string &Conference::filterString() const {
        static const std::string filter =
                "/iq/conference[@xmlns='" + JITSI_CONFERENCE + "']";
        return filter;
    }

    ConferenceManager::ConferenceManager(ClientBase *parent, ConferenceHandler *ch)
            : m_parent(parent), m_handler(ch) {
        m_parent->registerIqHandler(this, ExtConference);
        m_parent->disco()->addFeature(JITSI_CONFERENCE);
        m_parent->registerStanzaExtension(new Conference());
    }

    ConferenceManager::~ConferenceManager() {}

    bool ConferenceManager::handleIq(const IQ &iq) {

        const Conference *j = iq.findExtension<Conference>(ExtConference);
        if (!j || !m_handler || !m_parent)

            return false;

        m_handler->onStart(j);
        return true;
    }

    void ConferenceManager::handleIqID(const IQ &iq, int context) {
    }

    void ConferenceManager::create(const JID &room,               //
                                   const JID &focus,              //
                                   const std::string &machineUid, //
                                   ConferenceHandler *handler) {
        m_handler = handler;

        IQ iq(IQ::IqType::Set, focus, m_parent->getID());
        auto t = iq.tag();

//        auto c = new Tag("conference", "xmlns", JITSI_CONFERENCE);
//        c->addAttribute("room", room.bare());
//        c->addAttribute("machine-uid", machineUid);
//        auto p1 = new Tag("property", "name", "disableRtx");
//        p1->addAttribute("value", "false");
//        auto p2 = new Tag("property", "name", "enableLipSync");
//        p2->addAttribute("value", "true");
//        auto p3 = new Tag("property", "name", "openSctp");
//        p3->addAttribute("value", "true");
        //  iq to='focus.meet.chuanshaninfo.com'
        //  id='17b560524b01f8f741442d88e18b9a844e8b87d50000001e'
        //  type='set'
        //  from='18510248810@meet.chuanshaninfo.com/OkEDU-Classroom-Desktop'
        //  xmlns='jabber:client'>
        //  <conference xmlns='http://jitsi.org/protocol/focus'
        //  room='280300a4-av@conference.meet.chuanshaninfo.com'
        //  machine-uid='{c3b17767-342f-460d-a3e4-5a32753af1aa}'>
        //  <property name='disableRtx' value='false'/>
        //  <property name='enableLipSync' value='true'/>
        //  <property name='openSctp' value='true'/>
        //  </conference></iq>

        iq.addExtension(new gloox::Conference(focus,       //
                                              room.full(), //
                                              machineUid,  //
                                              false, true, true));

        m_parent->send(iq);
    }

} // namespace gloox
