//
// Created by gaojie on 24-11-25.
//


#include "gloox.h"
#include "iqhandler.h"
#include "clientbase.h"
#include "meethandler.h"
#include "presencehandler.h"


#ifndef MEET_MANAGER_H
#define MEET_MANAGER_H

namespace gloox {
    static const std::string MEET_FOCUS = "focus";
    class GLOOX_API MeetManager : public IqHandler, public PresenceHandler {
    public:
        explicit MeetManager(ClientBase *parent);

        ~MeetManager() override;

        void registerHandler(MeetHandler *handler);

        void createMeet(const Meet &meet);

        void exitMeet(const JID &jid);

    protected:
        //IqHandler
        bool handleIq(const IQ &iq) override;

        void handleIqID(const IQ &iq, int context) override;

        //PresenceHandler
        void handlePresence(const Presence &presence) override;


    private:
        MeetHandler *m_handler;
        ClientBase *m_parent;
    };

} // gloox

#endif //CONFERENCEMANAGER_H
