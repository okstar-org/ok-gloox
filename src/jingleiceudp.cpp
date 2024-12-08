/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "jingleiceudp.h"

#include "gloox.h"
#include "tag.h"

namespace gloox {

    namespace Jingle {

        static const char *typeValues[] = {"host", "prflx", "relay", "srflx"};

        SignalingWebRTCDatachannels::SignalingWebRTCDatachannels() {}

        SignalingWebRTCDatachannels::SignalingWebRTCDatachannels(
                const int &port, const int &bit, const std::string &name)
                : m_port(port), m_bit(bit), m_name(name) {}

        ICEUDP::ICEUDP(const std::string &pwd, const std::string &ufrag,
                       const CandidateList &candidates)
                : Plugin(PluginICEUDP), m_pwd(pwd), m_ufrag(ufrag),
                  m_candidates(candidates) {}

        ICEUDP::ICEUDP(const Tag *tag) : Plugin(PluginICEUDP) {
            if (!tag || tag->name() != "transport" ||
                tag->xmlns() != XMLNS_JINGLE_ICE_UDP)
                return;

            m_pwd = tag->findAttribute("pwd");
            m_ufrag = tag->findAttribute("ufrag");
            const TagList candidates = tag->findChildren("candidate");
            TagList::const_iterator it = candidates.begin();
            for (; it != candidates.end(); ++it) {
                Candidate c;
                c.component = std::stoi((*it)->findAttribute("component"));
                c.foundation = (*it)->findAttribute("foundation");
                c.generation = std::stoi((*it)->findAttribute("generation"));
                c.id = (*it)->findAttribute("id");
                c.ip = (*it)->findAttribute("ip");
                c.network = std::stoi((*it)->findAttribute("network"));
                c.port = atoi((*it)->findAttribute("port").c_str());
                c.priority = atoi((*it)->findAttribute("priority").c_str());
                c.protocol = (*it)->findAttribute("protocol");
                c.rel_addr = (*it)->findAttribute("rel-addr");
                c.rel_port = atoi((*it)->findAttribute("rel-port").c_str());
                c.type = static_cast<Type>(
                        util::lookup((*it)->findAttribute("type"), typeValues));
                m_candidates.push_back(c);
            }

            const TagList fingerprints = tag->findChildren("fingerprint");
            it = fingerprints.begin();
            for (; it != fingerprints.end(); ++it) {
                Dtls dtls;
                dtls.hash = (*it)->findAttribute("hash");
                dtls.setup = (*it)->findAttribute("setup");
                dtls.fingerprint = (*it)->cdata();
                m_dtls = (dtls);
            }
        }

        const StringList ICEUDP::features() const {
            StringList sl;
            sl.push_back(XMLNS_JINGLE_ICE_UDP);
            return sl;
        }

        const std::string &ICEUDP::filterString() const {
            static const std::string filter =
                    "content/transport[@xmlns='" + XMLNS_JINGLE_ICE_UDP + "']";
            return filter;
        }

        Plugin *ICEUDP::newInstance(const Tag *tag) const { return new ICEUDP(tag); }

        Tag *ICEUDP::tag() const {
            Tag *t = new Tag("transport", XMLNS, XMLNS_JINGLE_ICE_UDP);
            t->addAttribute("pwd", m_pwd);
            t->addAttribute("ufrag", m_ufrag);

            CandidateList::const_iterator it = m_candidates.begin();
            for (; it != m_candidates.end(); ++it) {
                Tag *c = new Tag(t, "candidate");
                c->addAttribute("component", (*it).component);
                c->addAttribute("foundation", (*it).foundation);
                c->addAttribute("generation", std::to_string((*it).generation));
                c->addAttribute("id", (*it).id);
                c->addAttribute("ip", (*it).ip);
                c->addAttribute("network",std::to_string( (*it).network));
                c->addAttribute("port",std::to_string( (*it).port));
                c->addAttribute("priority",std::to_string( (*it).priority));
                c->addAttribute("protocol", (*it).protocol);
                c->addAttribute("type", util::lookup((*it).type, typeValues));
                if ((*it).type != Host) {
                    c->addAttribute("rel-addr", (*it).rel_addr);
                    c->addAttribute("rel-port",std::to_string( (*it).rel_port));
                }
            }

            Tag *c = new Tag(t, "fingerprint");
            c->setXmlns(XMLNS_JINGLE_APPS_DTLS);
            c->addAttribute("hash", m_dtls.hash);
            c->addAttribute("setup", m_dtls.setup);
            c->setCData(m_dtls.fingerprint);

            return t;
        }

        const SignalingWebRTCDatachannels &ICEUDP::signal_webrtc_datachannels() const {
            return signalingWebRTCDatachannels;
        }

        void ICEUDP::setSignalingWebRTCDatachannels(
                const SignalingWebRTCDatachannels &_signalingWebRTCDatachannels) {
            signalingWebRTCDatachannels = _signalingWebRTCDatachannels;
        }

        void ICEUDP::setDtls(const ICEUDP::Dtls &dtls) {
            m_dtls = dtls;
        }
    } // namespace Jingle

} // namespace gloox
