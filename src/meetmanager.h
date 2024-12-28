//
// Created by gaojie on 24-11-25.
//


#include "gloox.h"
#include "iqhandler.h"
#include "clientbase.h"
#include "meethandler.h"
#include "presencehandler.h"
#include "messagehandler.h"

#ifndef MEET_MANAGER_H
#define MEET_MANAGER_H

namespace gloox {
    static const std::string MEET_FOCUS = "focus";

    class GLOOX_API MeetManager : public IqHandler, public MessageHandler, public PresenceHandler {
    public:
        explicit MeetManager(ClientBase *parent);

        ~MeetManager() override;

        void registerHandler(MeetHandler *handler);

        Meet *createMeet(const JID &room, const std::string& resource, std::map<std::string, std::string> &props);

        void exitMeet();

        void join(Meet &meet, const Meet::Participant &participant);

    protected:
        //IqHandler
        bool handleIq(const IQ &iq) override;

        void handleIqID(const IQ &iq, int context) override;

        //PresenceHandler
        void handlePresence(const Presence &presence) override;

        void handleMessage(const Message &msg, MessageSession *session = 0) override;

    private:
        MeetHandler *m_handler;
        ClientBase *m_parent;
        Meet *m_meet;
    };

} // gloox

#endif //CONFERENCEMANAGER_H
