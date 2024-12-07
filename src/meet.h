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
            //self jid
            MUCRoom::MUCUser mucUser;
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

        bool addParticipant(const Participant &p) {
            if (p.resource.empty())
                return false;

            Participants::const_iterator it = participants.find(p.resource);
            if (it != participants.end()) {
                return false;
            }

            participants.insert(std::make_pair(p.resource, p));
            return true;

            // 使用 try_emplace 方法尝试插入元素
//            auto result = participants.try_emplace(p.resource, p);
//            // 检查是否成功插入新元素
//            if (result.second) {
//                return true;
//            }
//            return false;
        }

    private:
        JID m_jid;
        std::string uid;
        std::map<std::string, std::string> properties;
        Participants participants;
    };
} // namespace gloox

#endif // CONFERENCE_H
