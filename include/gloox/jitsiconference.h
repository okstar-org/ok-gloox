/**
* Copyright (c) 2022 船山科技 chuanshantech.com
OkEDU-Classroom is licensed under Mulan PubL v2.
You can use this software according to the terms and conditions of the Mulan
PubL v2. You may obtain a copy of Mulan PubL v2 at:
        http://license.coscl.org.cn/MulanPubL-2.0
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PubL v2 for more details.
*/

#ifndef CONFERENCE_H
#define CONFERENCE_H

#include "client.h"
#include "stanzaextension.h"

namespace gloox {


    class Conference : public StanzaExtension {
    public:

        Conference(const JID &focus, const std::string &room,
                   const std::string &machineUid,
                   const bool disableRtx = false,
                   const bool enableLipSync = true,
                   const bool openSctp = true);

        Conference();

        Conference(const Tag *tag);

        Conference(const Conference &conference);

        virtual Tag *tag() const;

        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new Conference(tag);
        }

        virtual StanzaExtension *clone() const { return new Conference(*this); }

        bool disableRtc() { return m_disableRtx; }

        bool enableLipSync() { return m_enableLipSync; }

        bool openSctp() { return m_openSctp; }

        bool ready() { return m_ready; }

        const JID &focusJid() { return m_focusJid; }

        bool auth() { return m_auth; }

        const std::string &room() { return m_room; }

    private:
        JID m_focus;
        std::string m_room;
        std::string m_machineUid;

        bool m_disableRtx;
        bool m_enableLipSync;
        bool m_openSctp;

        //返回值
        bool m_ready;
        JID m_focusJid;
        bool m_auth;

    };

    class GLOOX_API ConferenceHandler {
    public:
        /**
         * Virtual destructor.
         */
        virtual ~ConferenceHandler() {}

        virtual void onStart(const Conference *j) = 0;
    };

    class GLOOX_API ConferenceManager : public IqHandler {
    public:
        ConferenceManager(ClientBase *parent, ConferenceHandler *ch);

        virtual ~ConferenceManager();

        void create(const JID &jid, const JID &focus, const std::string &machineUid,
                    ConferenceHandler *handler);

        // reimplemented from IqHandler
        virtual bool handleIq(const IQ &iq) override;

        // reimplemented from IqHandler
        virtual void handleIqID(const IQ & /*iq*/, int /*context*/) override;

    private:
        ClientBase *m_parent;
        ConferenceHandler *m_handler;
    };

} // namespace gloox

#endif // CONFERENCE_H
