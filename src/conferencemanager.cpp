//
// Created by gaojie on 24-11-25.
//

#include "conferencemanager.h"
#include "disco.h"

namespace gloox {
    ConferenceManager::ConferenceManager(ClientBase *parent) : m_handler(nullptr), m_parent(parent) {
        if (m_parent) {
            m_parent->disco()->addFeature( XMLNS_JITSI_FOCUS );
            m_parent->registerIqHandler(this, ExtJitsiConference);
        }
    }

    ConferenceManager::~ConferenceManager(){
        if(m_parent){
            m_parent->removeIqHandler(this, ExtJitsiConference);
        }
    }

    void ConferenceManager::registerHandler(gloox::ConferenceHandler *handler) {
        m_handler = handler;
    }


} // gloox