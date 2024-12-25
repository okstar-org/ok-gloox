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
#include "mucroom.h"

namespace gloox {
    class GLOOX_API Meet : public StanzaExtension {
    public:
        struct Participant {
            std::string region;
            std::string codecType;
            //data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53M...
            std::string avatarUrl;
            std::string email;
            std::string nick;
            std::string resource;
            std::string affiliation;
            std::string role;
            JID jid;
            std::string sourceInfo;
            bool e2ee;
            std::map<std::string, std::string> idKeys;
            std::string stats_id;

        };

        Tag* tagParticipant(const Participant& p, Presence &presence) const;

        Participant parseParticipant(const gloox::JID &from, const gloox::Presence &presence);

        typedef std::map<std::string, Participant> Participants;

        explicit Meet(ClientBase *parent, const JID &m_jid, const std::string &uid,
                      const std::map<std::string, std::string> &properties);

        explicit Meet(const Tag *tag);

        const std::string &getUid() const {
            return uid;
        }

        virtual Tag *tag() const override;

        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new Meet(tag);
        }

        virtual StanzaExtension *clone() const { return new Meet(*this); }

        inline const JID &jid() const { return m_jid; }

        inline const std::map<std::string, std::string> &getProperties() const {
            return properties;
        }

        inline const Participants &getParticipants() const {
            return participants;
        }

        bool addParticipant(const Participant &p);

        inline const Participant &getSelf() const {
            return self;
        }

        void send(const std::string &msg);

        void sendPresence(const Participant &self);

    private:
        JID m_jid;
        std::string uid;
        std::map<std::string, std::string> properties;
        Participants participants;
        Participant self;
        ClientBase *m_parent;
    };
} // namespace gloox

#endif // CONFERENCE_H
