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

#include "jingleibb.h"

#include "gloox.h"
#include "tag.h"

namespace gloox {

    namespace Jingle {


        IBB::IBB(const std::string &sid, long blockSize)
                : Plugin(PluginIBB), m_sid(sid), m_blockSize(blockSize) //
        {}

        IBB::IBB(const Tag *tag) : Plugin(PluginIBB) {
            if (!tag || tag->xmlns() != XMLNS_JINGLE_IBB)
                return;

            if (tag->name() == "transport") {
                m_action = init;
                /**
                 *  <transport xmlns='urn:xmpp:jingle:transports:ibb:1'
                               block-size='4096'
                               sid='ch3d9s71'/>
                 */
                m_sid = tag->findAttribute("sid");
                m_blockSize = std::strtol(tag->findAttribute("block-size").data(), NULL, 0);
            }

            if (tag->name() == "open") {
                /**
                 * <open sid='wmEBp4Xszb7WbkfPMLeI2w'
                 * xmlns='http://jabber.org/protocol/ibb'
                 * stanza='iq' block-size='8192'/>
                 */
                m_action = open;
                m_blockSize = std::strtol(tag->findAttribute("block-size").data(), NULL, 0);
            }

            if (tag->name() == "data") {
                /**
                 *  <data xmlns='http://jabber.org/protocol/ibb' seq='0' sid='ch3d9s71'>
    qANQR1DBwU4DX7jmYZnncmUQB/9KuKBddzQH+tZ1ZywKK0yHKnq57kWq+RFtQdCJ
    WpdWpR0uQsuJe7+vh3NWn59/gTc5MDlX8dS9p0ovStmNcyLhxVgmqS8ZKhsblVeu
    IpQ0JgavABqibJolc3BKrVtVV1igKiX/N7Pi8RtY1K18toaMDhdEfhBRzO/XB0+P
    AQhYlRjNacGcslkhXqNjK5Va4tuOAPy2n1Q8UUrHbUd0g+xJ9Bm0G0LZXyvCWyKH
    kuNEHFQiLuCY6Iv0myq6iX6tjuHehZlFSh80b5BVV9tNLwNR5Eqz1klxMhoghJOA
  </data>
                 */
                m_action = data;
                m_sid = tag->findAttribute("sid");
                m_data.seq = std::strtol(tag->findAttribute("seq").data(), NULL, 0);
                m_data.chunk = tag->cdata();
            }
        }

        const StringList IBB::features() const {
            StringList sl;
            sl.push_back(XMLNS_JINGLE_IBB);
            return sl;
        }

        const std::string &IBB::filterString() const {
            static const std::string filter =
                    "content/transport[@xmlns='" + XMLNS_JINGLE_IBB + "']" +
                    "|open[@xmlns='" + XMLNS_JINGLE_IBB + "']" +
                    "|data[@xmlns='" + XMLNS_JINGLE_IBB + "']";
            return filter;
        }

        Plugin *IBB::newInstance(const Tag *tag) const { return new IBB(tag); }

        Tag *IBB::tag() const {
            Tag *t = new Tag("transport", XMLNS, XMLNS_JINGLE_IBB);
            t->addAttribute("sid", m_sid);
            t->addAttribute("block-size", m_blockSize);
            return t;
        }

    } // namespace Jingle
} // namespace gloox
