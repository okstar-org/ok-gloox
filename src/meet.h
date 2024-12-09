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


    class Meet : public StanzaExtension {
    public:
        struct SourceInfo {

        };

        struct Participant {
            std::string region;
            std::string codecType;
            //data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53M...
            std::string avatarUrl;
            std::string email;
            std::string nick;
            std::string resource;
            bool e2ee;
            std::map<std::string, std::string> idKeys;
        };

        typedef std::map<std::string, Participant> Participants;

        explicit Meet();

        explicit Meet(const JID &m_jid, const std::string &uid, const std::map<std::string, std::string> &properties);

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

        const JID &jid() const { return m_jid; }

        const std::map<std::string, std::string> &getProperties() const {
            return properties;
        }

        inline const Participants &getParticipants() const {
            return participants;
        }

        bool addParticipant(const Participant &p);

    private:
        JID m_jid;
        std::string uid;
        std::map<std::string, std::string> properties;
        Participants participants;
    };
} // namespace gloox

#endif // CONFERENCE_H
