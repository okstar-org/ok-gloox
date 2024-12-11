/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "jinglertp.h"
#include <iostream>

namespace gloox {
    namespace Jingle {

        RTP::RTP(const Tag *tag) : Plugin(PluginRTP),
                                   m_media(invalid),
                                   m_rtcpMux(false) {
            if (!tag || tag->name() != "description" ||
                tag->xmlns() != XMLNS_JINGLE_APPS_RTP)
                return;

            const std::string &tagMedia = tag->findAttribute("media");
            if (tagMedia == "audio") {
                m_media = audio;
            } else if (tagMedia == "video") {
                m_media = video;
            } else if (tagMedia == "application") {
                m_media = application;
            }

            const Tag *c = tag->findTag("description/payload-type");
            if (c) {
                parsePayloadTypes(tag->findChildren("payload-type"));
            }

            const Tag *h = tag->findTag("description/rtp-hdrext");
            if (h) {
                parseHdrExts(tag->findChildren("rtp-hdrext"));
            }

            const Tag *mux = tag->findTag("description/rtcp-mux");
            if (mux) {
                m_rtcpMux = true;
            }

            const Tag *source = tag->findTag("description/source");
            if (source) {
                parseSources(tag->findChildren("source"));
            }

            const Tag *ssrcGroup = tag->findTag("description/ssrc-group");
            if (ssrcGroup) {
                parseSsrcGroups(ssrcGroup);
            }
        }

        RTP::RTP(Media media, const PayloadTypes &payloadTypes)
                : Plugin(PluginRTP), m_payloadTypes(payloadTypes), m_media(media) {}

        const gloox::StringList RTP::features() const {
            StringList sl;
            sl.push_back(XMLNS_JINGLE_APPS_RTP);
            return sl;
        }

        const std::string &RTP::filterString() const {
            static const std::string filter =
                    "content/description[@xmlns='" + XMLNS_JINGLE_APPS_RTP + "']";
            return filter;
        }

        Plugin *RTP::newInstance(const Tag *tag) const { return new RTP(tag); }

        Tag *RTP::tag() const {
            if (m_media == invalid)
                return NULL;

            Tag *r = new Tag("description", XMLNS, XMLNS_JINGLE_APPS_RTP);

            switch (m_media) {
                case audio: {
                    r->addAttribute("media", "audio");
                    break;
                }
                case video: {
                    r->addAttribute("media", "video");
                    break;
                }
                case application:
                    r->addAttribute("media", "application");
                    break;
              default:
                break;
            }

            addPayloadTypes(r);
            addHdrExts(r);
            addSources(r);
            addSsrcGroup(r);

            if (m_rtcpMux) {
                new Tag(r, "rtcp-mux");
            }

            return r;
        }

        void RTP::addSsrcGroup(Tag *r) const {
            if(m_ssrcGroup.ssrcs.empty())
                return;
            Tag *f = new Tag(r, "ssrc-group");
            f->setXmlns(XMLNS_JINGLE_APPS_RTP_SSMA);
            f->addAttribute("semantics", m_ssrcGroup.semantics);
            std::vector<std::string>::const_iterator sit = m_ssrcGroup.ssrcs.begin();
            for (; sit != m_ssrcGroup.ssrcs.end(); ++sit) {
                Tag *pt = new Tag(f, "source");
                pt->addAttribute("ssrc", (*sit));
            }
        }

        void RTP::addSources(Tag *r) const {
            Sources::const_iterator it = m_sources.begin();
            for (; it != m_sources.end(); ++it) {
                Tag *f = new Tag(r, "source");
                f->setXmlns(XMLNS_JINGLE_APPS_RTP_SSMA);
                f->addAttribute("ssrc", (*it).ssrc);
                Parameters::const_iterator it2 = (*it).parameters.begin();
                for (; it2 != (*it).parameters.end(); ++it2) {
                    Tag *pt = new Tag(f, "parameter");
                    pt->addAttribute("name", (*it2).name);
                    pt->addAttribute("value", (*it2).value);
                }
            }
        }

        void RTP::addHdrExts(Tag *r) const {
            HdrExts::const_iterator it = m_hdrExts.begin();
            for (; it != m_hdrExts.end(); ++it) {
                Tag *f = new Tag(r, "rtp-hdrext");
                f->setXmlns(XMLNS_JINGLE_APPS_RTP_HDREXT);
                f->addAttribute("id", (*it).id);
                f->addAttribute("uri", (*it).uri);
            }
        }

        void RTP::addPayloadTypes(Tag *r) const {
            PayloadTypes::const_iterator it = m_payloadTypes.begin();
            for (; it != m_payloadTypes.end(); ++it) {
                Tag *f = new Tag(r, "payload-type");
                f->addAttribute("id", (*it).id);
                f->addAttribute("name", (*it).name);
                if ((*it).clockrate > 0) {
                    f->addAttribute("clockrate", ((*it).clockrate));
                }
                if ((*it).bitrate > 0) {
                    f->addAttribute("bitrate", ((*it).bitrate));
                }

                if ((*it).channels > 1) {
                    f->addAttribute("channels", (*it).channels);
                }

                Parameters::const_iterator it2 = (*it).parameters.begin();
                for (; it2 != (*it).parameters.end(); ++it2) {
                    Tag *pt = new Tag(f, "parameter");
                    pt->addAttribute("name", (*it2).name);
                    pt->addAttribute("value", (*it2).value);
                }

                Feedbacks::const_iterator it3 = (*it).feedbacks.begin();
                for (; it3 != (*it).feedbacks.end(); ++it3) {
                    Tag *pt = new Tag(f, "rtcp-fb");
                    pt->setXmlns(XMLNS_JINGLE_APPS_RTP_FB);
                    pt->addAttribute("type", (*it3).type);
                    if (!(*it3).subtype.empty())
                        pt->addAttribute("subtype", (*it3).subtype);
                }
            }
        }


        RTP::Parameters RTP::parseParameters(const TagList &tagList) {
            Parameters ps;
            TagList::const_iterator it = tagList.begin();
            for (; it != tagList.end(); ++it) {
                Parameter f;
                f.name = (*it)->findAttribute("name");
                f.value = (*it)->findAttribute("value");
                ps.push_back(f);
            }
            return ps;
        }

        void RTP::parseSources(const TagList &tagList) {
            TagList::const_iterator it = tagList.begin();
            for (; it != tagList.end(); ++it) {
                Source f;
                f.ssrc = (*it)->findAttribute("ssrc").data();
                f.parameters = parseParameters((*it)->findChildren("parameter"));
                m_sources.push_back(f);
            }
        }

        void RTP::parseSsrcGroups(const Tag *ssrcGroup) {
            /**
             *  <ssrc-group xmlns="urn:xmpp:jingle:apps:rtp:ssma:0" semantics="FID">
                  <source ssrc="1547916473"/>
                  <source ssrc="855138671"/>
                </ssrc-group>
             */
            m_ssrcGroup.semantics = ssrcGroup->findAttribute("semantics").data();
            TagList tagList = ssrcGroup->findChildren("source");
            TagList::const_iterator it = tagList.begin();
            for (; it != tagList.end(); ++it) {
                m_ssrcGroup.ssrcs.push_back((*it)->findAttribute("ssrc").data());
            }
        }

        void RTP::parseHdrExts(const TagList &hdrExts) {
            TagList::const_iterator it = hdrExts.begin();
            for (; it != hdrExts.end(); ++it) {
                HdrExt f;
                f.id = std::strtol((*it)->findAttribute("id").data(), NULL, 10);
                f.uri = (*it)->findAttribute("uri");
                m_hdrExts.push_back(f);
            }
        }

        void RTP::parsePayloadTypes(const TagList &payloadTypes) {
            TagList::const_iterator it = payloadTypes.begin();
            for (; it != payloadTypes.end(); ++it) {
                PayloadType f;

                f.name = (*it)->findAttribute("name");
                f.id = std::strtol((*it)->findAttribute("id").data(), NULL, 10);

                std::string clockrate = (*it)->findAttribute("clockrate");
                f.clockrate = !clockrate.empty() ? std::strtol(clockrate.data(), NULL, 10) : 0;

                std::string bitrate = (*it)->findAttribute("bitrate");
                f.bitrate = !bitrate.empty() ? std::strtol(bitrate.data(), NULL, 10) : 0;

                std::string channels = (*it)->findAttribute("channels");
                f.channels = !channels.empty() ? std::strtol(channels.data(), NULL, 10) : 1;

                TagList parameters = (*it)->findChildren("parameter");
                TagList::const_iterator pit = parameters.begin();
                for (; pit != parameters.end(); ++pit) {
                    Parameter p;
                    p.name = (*pit)->findAttribute("name");
                    p.value = (*pit)->findAttribute("value");
                    f.parameters.push_back(p);
                }

                Tag *fbTag = (*it)->findChild("rtcp-fb");
                if (fbTag) {
                    Feedback p;
                    p.type = fbTag->findAttribute("type");
                    p.subtype = fbTag->findAttribute("subtype");
                    f.feedbacks.push_back(p);
                }

                m_payloadTypes.push_back(f);
            }
        }

        Media RTP::media() const { return m_media; }

        void RTP::setMedia(Media m) { m_media = m; }

        const RTP::PayloadTypes &RTP::payloadTypes() const { return m_payloadTypes; }

        const RTP::HdrExts &RTP::hdrExts() const { return m_hdrExts; }

        bool RTP::rtcpMux() const { return m_rtcpMux; }

        const RTP::Sources &RTP::sources() const { return m_sources; }

        const RTP::SsrcGroup &RTP::ssrcGroup() const {
            return m_ssrcGroup;
        }

        void RTP::setPayloadTypes(const PayloadTypes &payloadTypes) {
            m_payloadTypes = payloadTypes;
        }

        void RTP::setRtcpMux(bool rtcpMux) { m_rtcpMux = rtcpMux; }

        void RTP::setHdrExts(const RTP::HdrExts &hdrExts) { m_hdrExts = hdrExts; }

        void RTP::setSources(const RTP::Sources &sources) { m_sources = sources; }

        void RTP::setSsrcGroup(const RTP::SsrcGroup &ssrcGroup) {
            m_ssrcGroup = ssrcGroup;
        }
    } // namespace Jingle
} // namespace gloox
