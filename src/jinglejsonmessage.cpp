/*
Copyright (c) 2022 船山科技 chuanshantech.com
OkEDU-Classroom is licensed under Mulan PubL v2.
You can use this software according to the terms and conditions of the Mulan PubL v2.
You may obtain a copy of Mulan PubL v2 at:
         http://license.coscl.org.cn/MulanPubL-2.0
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PubL v2 for more details.

Mulan Public License，Version 2

Mulan Public License，Version 2 (Mulan PubL v2)

May 2021 http://license.coscl.org.cn/MulanPubL-2.0

Your reproduction, use, modification and Distribution of the Contribution shall be subject to Mulan Public License, Version 2 (this License) with following terms and conditions:

*/

#include "jinglejsonmessage.h"

#include "gloox.h"
#include "tag.h"

namespace gloox {
    namespace Jingle {

        const std::string JsonMessage::TAG_NAME = "json-message";

        JsonMessage::JsonMessage(const std::string &json)
                : Plugin(PluginJsonMessage), m_json(json) {}

        JsonMessage::JsonMessage(const Tag *tag) : Plugin(PluginJsonMessage) {
            if (!tag || tag->xmlns() != XMLNS_JIT_MEET)
                return;

            if (tag->name() == TAG_NAME) {
                /**
                 *  <json-message xmlns='http://jitsi.org/jitmeet'>
                 *      { json content }
                 *  </json-message>
                 */
                m_json = tag->cdata();
            }
        }

        const StringList JsonMessage::features() const {
            StringList sl;
            sl.push_back(XMLNS_JIT_MEET);
            return sl;
        }

        const std::string &JsonMessage::filterString() const {
            static std::string f = "jingle/"+TAG_NAME+"|"+TAG_NAME;
            return f;
        }

        Plugin *JsonMessage::newInstance(const Tag *tag) const { return new JsonMessage(tag); }

        Tag *JsonMessage::tag() const {
            Tag *t = new Tag(TAG_NAME, XMLNS, XMLNS_JIT_MEET);
            t->setCData(m_json);
            return t;
        }
    } // namespace Jingle
} // namespace gloox
