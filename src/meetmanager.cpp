//
// Created by gaojie on 24-11-25.
//

#include "meetmanager.h"
#include "capabilities.h"
#include "disco.h"
#include "meet.h"
#include "util.h"
#include "nickname.h"
#include "message.h"
#include "jsonmessage.h"

namespace gloox {
    MeetManager::MeetManager(ClientBase *parent)
            : m_handler(nullptr), m_parent(parent) {
        if (m_parent) {
            m_parent->disco()->addFeature(XMLNS_JITSI_FOCUS);
            m_parent->registerStanzaExtension(new Meet());
            m_parent->registerStanzaExtension(new JsonMessage());
            m_parent->registerIqHandler(this, ExtMeet);
            m_parent->registerMessageHandler(this);
        }
    }

    MeetManager::~MeetManager() {
        if (m_parent) {
            m_parent->removeStanzaExtension(ExtMeet);
            m_parent->removeIqHandler(this, ExtMeet);
        }
    }

    void MeetManager::registerHandler(gloox::MeetHandler *handler) {
        m_handler = handler;
    }

    bool MeetManager::handleIq(const IQ &iq) {
        const Meet *c = iq.findExtension<Meet>(ExtMeet);
        if (c && m_handler) {
            std::map<std::string, std::string>::const_iterator f =
                    c->getProperties().find("ready");
            bool ready = false;
            if (f != c->getProperties().end()) {
                ready = f->second == "true";
            }
            m_handler->handleCreation(c->jid(), ready, c->getProperties());
        }
        return true;
    }

    void MeetManager::createMeet(const Meet &meet) {
        if (!m_parent)
            return;

        Tag *tag = meet.tag();
        if (!tag)
            return;

        JID to(MEET_FOCUS + "." + m_parent->server());
        IQ iq(IQ::Set, to, m_parent->getID());

        Tag *a = iq.tag();
        a->setXmlns(XMLNS_CLIENT);
        a->addChild(tag);
        m_parent->send(a);
    }

    void MeetManager::exitMeet(const JID &jid) {
        if (!m_parent) {
            return;
        }

        Presence un(Presence::Unavailable, jid, XMLNS_CLIENT);
        m_parent->send(un);
    }


    void MeetManager::handleIqID(const IQ &iq, int context) {}

    void MeetManager::handlePresence(const Presence &presence) {
        if (presence.subtype() == Presence::Error)
            return;


        Tag *t = presence.tag();
        if (!t)
            return;

        if (!m_handler) {
            return;
        }


        const Capabilities *c = presence.capabilities();
        if (c->node() == XMLNS_JITSI_MEET) {
            Meet::Participant participant;
            participant.region = t->findChild("jitsi_participant_region")->cdata();
            participant.codecType = t->findChild("jitsi_participant_codecType")->cdata();
            participant.avatarUrl = t->findChild("avatar-url")->cdata();
            participant.email = t->findChild("email")->cdata();
            participant.nick = t->findChild("nick")->cdata();

            const JID &jid = presence.from();
            m_handler->handleParticipant(jid, participant);
            m_handler->handleStatsId(jid, t->findChild("stats-id")->cdata());
        }
    }

    void MeetManager::handleMessage(const Message &msg, MessageSession *session) {
        if (!m_handler) {
            return;
        }
        JsonMessage *t = (JsonMessage *) msg.findExtension(ExtMeetJsonMessage);
        if (t) {
            m_handler->handleJsonMessage(msg.from(), t);
        }
    }

    void MeetManager::join(Meet &meet, const Meet::Participant &participant) {
        bool yes = meet.addParticipant(participant);
        if (!yes)
            return;

        //presence
        JID to(meet.jid());
        to.setResource(participant.resource);
        Presence part(Presence::Available, to, EmptyString, 0, XMLNS_CLIENT);

        Tag *t = part.tag();

        Tag *x = new Tag("x", "xmlns", XMLNS_MUC);
        t->addChild(x);


        Tag *stats = new Tag("stats-id", "Norris-oDt");
        t->addChild(stats);
        // <c hash=\"sha-1\" node=\"https://jitsi.org/jitsi-meet\"
        //            ver=\"zI2SFRd7GspEplY5WO8C4+MeFgQ=\" xmlns=\"http://jabber.org/protocol/caps\"/>

        Tag *c = new Tag("c", XMLNS, XMLNS_CAPS);
        c->addAttribute("node", XMLNS_JITSI_MEET);
        c->addAttribute("ver", "zI2SFRd7GspEplY5WO8C4+MeFgQ=");
        c->addAttribute("hash", "sha-1");
        t->addChild(c);

        //<SourceInfo>{}</SourceInfo>
        Tag *sourceInfo = new Tag("SourceInfo", "{}");
        t->addChild(sourceInfo);

        t->addChild(new Tag("jitsi_participant_region", participant.region));
        t->addChild(new Tag("jitsi_participant_codecType", participant.codecType));
        t->addChild(new Tag("avatar-url", participant.avatarUrl));
        t->addChild(new Tag("email", participant.email));
        Nickname *nick = new Nickname(participant.nick);
        t->addChild(nick->tag());

        m_parent->send(t);
    }


} // namespace gloox