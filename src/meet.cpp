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

#include "meet.h"
#include "disco.h"
#include "util.h"
#include "message.h"

namespace gloox {

    Meet::Meet(ClientBase *parent, const gloox::JID &m_jid_, const std::string &uid_,
               const std::map<std::string, std::string> &properties_)
            : StanzaExtension(ExtMeet), m_jid(m_jid_), uid(uid_), properties(properties_), m_parent(parent) {
        m_valid = true;
    }

    Meet::Meet(const Tag *tag) : StanzaExtension(ExtMeet), m_parent(nullptr) {
        if (!(tag && tag->name() == "conference" && tag->xmlns() == XMLNS_JITSI_FOCUS))
            return;

        m_jid = JID(tag->findAttribute("room"));
        uid = tag->findAttribute("machine-uid");
        TagList props = tag->findChildren("property");
        TagList::const_iterator it = props.begin();
        for (; it != props.end(); ++it) {
            auto name = (*it)->findAttribute("name");
            if (!name.empty()) {
                properties.insert(std::make_pair(name, (*it)->findAttribute("value")));
            }
        }

        m_valid = true;
    }

    Tag *Meet::tag() const {
        if (!m_valid)
            return nullptr;

        Tag *x = new Tag("conference", XMLNS, XMLNS_JITSI_FOCUS);
        x->addAttribute("room", m_jid.full());
        x->addAttribute("machine-uid", uid);

        //properties
        std::map<std::string, std::string>::const_iterator it = properties.begin();
        for (; it != properties.end(); ++it) {
            Tag *t = new Tag("property");
            t->addAttribute("name", it->first);
            t->addAttribute("value", it->second);
            x->addChild(t);
        }

        return x;
    }

    const std::string &Meet::filterString() const {
        static const std::string filter = "/iq/conference[@xmlns='" + XMLNS_JITSI_FOCUS + "']";
        return filter;
    }

    bool Meet::addParticipant(const Meet::Participant &p) {

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

    void Meet::send(const std::string &msg) {
        Message m(gloox::Message::MessageType::Groupchat,
                  jid(),
                  msg);
        m_parent->send(m);
    }

    void Meet::sendPresence(const Meet::Participant &participant) {
        Presence p(Presence::Available, jid());
        auto t = tagParticipant(participant, p);
        m_parent->send(t);
    }

    Tag *Meet::tagParticipant(const Participant &p, Presence &presence) const {
        Tag *t = presence.tag();
        t->addChild(new Tag("email", p.email));

        if (!p.stats_id.empty()) {
            t->addChild(new Tag("stats-id", p.stats_id));
        }

        if (!p.region.empty()) {
            t->addChild(new Tag("jitsi_participant_region", p.region));
        }

        if (!p.codecType.empty()) {
            t->addChild(new Tag("jitsi_participant_codecType", p.codecType));
        }

        if (!p.nick.empty()) {
            Tag *n = new Tag("nick", "xmlns", XMLNS_NICKNAME);
            n->setCData(p.nick);
            t->addChild(n);
        }

        if (!p.avatarUrl.empty()) {
            t->addChild(new Tag("avatar-url", p.avatarUrl));
        }

        if (!p.sourceInfo.empty()) {
            t->addChild(new Tag("SourceInfo", p.sourceInfo));
        }

        return t;
    }

    Meet::Participant Meet::parseParticipant(const gloox::JID &from, const gloox::Presence &presence) {

        auto t = presence.getOriginTag();

        // 成员上线
        auto email = t->findChild("email");
        if (!email) {
            return {};
        }

        Meet::Participant p = {
                .region = t->findChild("jitsi_participant_region")->cdata(),
                .codecType = t->findChild("jitsi_participant_codecType")->cdata(),
                .avatarUrl = t->findChild("avatar-url")
                             ? t->findChild("avatar-url")->cdata()
                             : "",
                .email = email->cdata(),
                .nick = t->findChild("nick") ? t->findChild("nick")->cdata() : "",
                .resource = from.resource(),
                .jid = from.full(),
                .e2ee = false,
                .stats_id = t->findChild("stats-id")->cdata()};

        auto fts = t->findChild("features");
        if (fts) {
            auto e2ee = fts->findChild("feature", "var", "https://jitsi.org/meet/e2ee");
            if (e2ee) {
                p.e2ee = true;
                auto ed25519 = t->findChild("jitsi_participant_e2ee.idKey.ed25519");
                if (ed25519) {
                    p.idKeys.insert(std::make_pair("ed25519", ed25519->cdata()));
                }
                auto curve25519 = t->findChild("jitsi_participant_e2ee.idKey.curve25519");
                if (curve25519) {
                    p.idKeys.insert(
                            std::make_pair("curve25519", curve25519->cdata()));
                }
            }
        }

        // 获取群组用户jid
        auto userTag = t->findChild("x", "xmlns", "http://jabber.org/protocol/muc#user");
        if (userTag) {
            gloox::MUCRoom::MUCUser mucUser(userTag);
        }

        auto si = t->findChild("SourceInfo");
        if (si) {
            p.sourceInfo = si->cdata();
        }

        return p;
    }
} // namespace gloox
