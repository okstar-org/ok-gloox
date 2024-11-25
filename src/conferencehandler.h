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
        virtual bool handleMUCRoomCreation(bool ready, std::map<std::string, std::string> props) = 0;
    };
}

#endif //CONFERENCEHANDLER_H
