//
// Created by gaojie on 24-11-25.
//

#include "gloox.h"
#include "conference.h"

#ifndef CONFERENCEHANDLER_H
#define CONFERENCEHANDLER_H

namespace gloox {
    class GLOOX_API ConferenceHandler {
    public:
        virtual void handleCreation(const JID &jid, bool ready, std::map<std::string, std::string> props) = 0;

        virtual void handleParticipant(const Conference::Participant &participant) = 0;

        virtual void handleStatsId(const std::string& statsId) = 0;
    };
}

#endif //CONFERENCEHANDLER_H
