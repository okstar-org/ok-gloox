//
// Created by gaojie on 24-11-25.
//


#include "gloox.h"
#include "iqhandler.h"
#include "clientbase.h"


#ifndef CONFERENCEMANAGER_H
#define CONFERENCEMANAGER_H

namespace gloox {

    class ConferenceHandler;

    class GLOOX_API ConferenceManager : public IqHandler {
    public:
        ConferenceManager(ClientBase *parent);

        ~ConferenceManager();

        void registerHandler(ConferenceHandler *handler);

    private:
        ConferenceHandler *m_handler;
        ClientBase *m_parent;
    };

} // gloox

#endif //CONFERENCEMANAGER_H
