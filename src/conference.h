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
        struct SourceInfo {

        };

        struct Participant {
            std::string region;
            std::string codecType;
            std::string avatarUrl;
            std::string email;
            std::string nick;
        };


        Conference();

        Conference(const Tag *tag);


        virtual Tag *tag() const override;

        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new Conference(tag);
        }

        virtual StanzaExtension *clone() const { return new Conference(*this); }

        const JID &jid() const { return m_jid; }

        const std::map<std::string, std::string> getProperties() const {
            return properties;
        }

    private:
        JID m_jid;
        std::string uid;
        std::map<std::string, std::string> properties;
    };
} // namespace gloox

#endif // CONFERENCE_H
