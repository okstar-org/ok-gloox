//
// Created by gaojie on 24-11-25.
//


#include "gloox.h"
#include "iqhandler.h"
#include "clientbase.h"
#include "conferencehandler.h"
#include "presencehandler.h"


#ifndef CONFERENCEMANAGER_H
#define CONFERENCEMANAGER_H

namespace gloox {

    class GLOOX_API ConferenceManager : public IqHandler, public PresenceHandler {
    public:
        ConferenceManager(ClientBase *parent);

        ~ConferenceManager();


        void registerHandler(ConferenceHandler *handler);

        void createConference(const Conference &conference);

    protected:
        //IqHandler
        bool handleIq(const IQ &iq) override;

        void handleIqID(const IQ &iq, int context) override;

        //PresenceHandler
        void handlePresence(const Presence &presence) override;


    private:
        ConferenceHandler *m_handler;
        ClientBase *m_parent;
    };

} // gloox

#endif //CONFERENCEMANAGER_H
