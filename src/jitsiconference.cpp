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
    JitsiConference::JitsiConference() : StanzaExtension(ExtJitsiConference) {}

    JitsiConference::JitsiConference(const JID &focus, const std::string &room,
                           const std::string &machineUid, const bool disableRtx,
                           const bool enableLipSync, const bool openSctp)
            : StanzaExtension(ExtJitsiConference), m_focus(focus), m_room(room),
              m_machineUid(machineUid), m_disableRtx(disableRtx),
              m_enableLipSync(enableLipSync), m_openSctp(openSctp) {}

    JitsiConference::JitsiConference(const Tag *tag) : StanzaExtension(ExtJitsiConference) {
        if (!tag || tag->name() != "JitsiConference" || tag->xmlns() != JITSI_CONFERENCE)
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

    JitsiConference::JitsiConference(const JitsiConference &JitsiConference)
            : StanzaExtension(ExtJitsiConference), m_focus(JitsiConference.m_focus),
              m_room(JitsiConference.m_room), m_machineUid(JitsiConference.m_machineUid),
              m_disableRtx(JitsiConference.m_disableRtx),
              m_enableLipSync(JitsiConference.m_enableLipSync),
              m_openSctp(JitsiConference.m_openSctp) {}

    Tag *JitsiConference::tag() const {
        Tag *t = new Tag("JitsiConference", XMLNS, JITSI_CONFERENCE);
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

    const std::string &JitsiConference::filterString() const {
        static const std::string filter =
                "/iq/JitsiConference[@xmlns='" + JITSI_CONFERENCE + "']";
        return filter;
    }

    JitsiConferenceManager::JitsiConferenceManager(ClientBase *parent, JitsiConferenceHandler *ch)
            : m_parent(parent), m_handler(ch) {
        m_parent->registerIqHandler(this, ExtJitsiConference);
        m_parent->disco()->addFeature(JITSI_CONFERENCE);
        m_parent->registerStanzaExtension(new JitsiConference());
    }

    JitsiConferenceManager::~JitsiConferenceManager() {}

    bool JitsiConferenceManager::handleIq(const IQ &iq) {

        const JitsiConference *j = iq.findExtension<JitsiConference>(ExtJitsiConference);
        if (!j || !m_handler || !m_parent)

            return false;

        m_handler->onStart(j);
        return true;
    }

    void JitsiConferenceManager::handleIqID(const IQ &iq, int context) {
    }

    void JitsiConferenceManager::create(const JID &room,               //
                                   const JID &focus,              //
                                   const std::string &machineUid, //
                                   JitsiConferenceHandler *handler) {
        m_handler = handler;

        IQ iq(IQ::IqType::Set, focus, m_parent->getID());
        auto t = iq.tag();

//        auto c = new Tag("JitsiConference", "xmlns", JITSI_JitsiConference);
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
        //  <JitsiConference xmlns='http://jitsi.org/protocol/focus'
        //  room='280300a4-av@JitsiConference.meet.chuanshaninfo.com'
        //  machine-uid='{c3b17767-342f-460d-a3e4-5a32753af1aa}'>
        //  <property name='disableRtx' value='false'/>
        //  <property name='enableLipSync' value='true'/>
        //  <property name='openSctp' value='true'/>
        //  </JitsiConference></iq>

        iq.addExtension(new gloox::JitsiConference(focus,       //
                                              room.full(), //
                                              machineUid,  //
                                              false, true, true));

        m_parent->send(iq);
    }

} // namespace gloox
