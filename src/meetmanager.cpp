//
// Created by gaojie on 24-11-25.
//

#include "meetmanager.h"
#include "capabilities.h"
#include "disco.h"
#include "meet.h"

namespace gloox {
MeetManager::MeetManager(ClientBase *parent)
    : m_handler(nullptr), m_parent(parent) {
  if (m_parent) {
    m_parent->disco()->addFeature(XMLNS_JITSI_FOCUS);
    m_parent->registerStanzaExtension(new Meet());
    m_parent->registerIqHandler(this, ExtMeet);
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

  Meet::Participant participant = {
      .region = t->findChild("jitsi_participant_region")->cdata(),
      .codecType = t->findChild("jitsi_participant_codecType")->cdata(),
      .avatarUrl = t->findChild("avatar-url")->cdata(),
      .email = t->findChild("email")->cdata(),
      .nick = t->findChild("nick")->cdata()};
  m_handler->handleParticipant(participant);

  m_handler->handleStatsId(t->findChild("stats-id")->cdata());
}

} // namespace gloox