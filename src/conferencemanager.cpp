//
// Created by gaojie on 24-11-25.
//

#include "conference.h"
#include "conferencemanager.h"
#include "disco.h"
#include "capabilities.h"

namespace gloox {
    ConferenceManager::ConferenceManager(ClientBase *parent) : m_handler(nullptr), m_parent(parent) {
        if (m_parent) {
            m_parent->disco()->addFeature(XMLNS_JITSI_FOCUS);
            m_parent->registerIqHandler(this, ExtJitsiConference);
        }
    }

    ConferenceManager::~ConferenceManager() {
        if (m_parent) {
            m_parent->removeIqHandler(this, ExtJitsiConference);
        }
    }

    void ConferenceManager::registerHandler(gloox::ConferenceHandler *handler) {
        m_handler = handler;
    }

    bool ConferenceManager::handleIq(const IQ &iq) {
        const Conference *c = iq.findExtension<Conference>(ExtJitsiConference);
        if (c && m_handler) {
            std::map<std::string, std::string>::const_iterator f = c->getProperties().find("ready");
            bool ready = false;
            if (f != c->getProperties().end()) {
                ready = f->second == "true";
            }
            m_handler->handleCreation(c->jid(), ready, c->getProperties());
        }


        return true;
    }

    void ConferenceManager::createConference(const Conference &conference) {
        if (!m_parent)
            return;

        Tag *t = conference.tag();
        t->addAttribute("id", m_parent->getID());
        m_parent->send(t);
    }

    void ConferenceManager::handleIqID(const IQ &iq, int context) {

    }

    void ConferenceManager::handlePresence(const Presence &presence) {
        if (presence.subtype() == Presence::Error)
            return;

        const Capabilities *c = presence.capabilities();
        if (c->node() != XMLNS_JITSI_MEET) {
            return;
        }

        Tag *t = presence.tag();
        if (!t)
            return;

        if (!m_handler) {
            return;
        }

        Conference::Participant participant = {
                .region = t->findChild("jitsi_participant_region")->cdata(),
                .codecType = t->findChild("jitsi_participant_codecType")->cdata(),
                .avatarUrl = t->findChild("avatar-url")->cdata(),
                .email = t->findChild("email")->cdata(),
                .nick =  t->findChild("nick")->cdata()
        };
        m_handler->handleParticipant(participant);

        m_handler->handleStatsId(t->findChild("stats-id")->cdata());
    }

} // gloox