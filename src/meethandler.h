//
// Created by gaojie on 24-11-25.
//

#include "gloox.h"
#include "meet.h"
#include "jsonmessage.h"

#ifndef MEET_HANDLER_H
#define MEET_HANDLER_H

namespace gloox {
    class GLOOX_API MeetHandler {
    public:
        virtual void handleCreation(const JID &jid, bool ready, const std::map<std::string, std::string>& props) = 0;

        virtual void handleParticipant(const Meet::Participant &participant) = 0;

        virtual void handleStatsId(const std::string &statsId) = 0;

        virtual void handleJsonMessage(const JsonMessage* message)  = 0;

    };
}

#endif //MEET_HANDLER_H
