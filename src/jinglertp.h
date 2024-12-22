/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef JINGLE_RTP_H__
#define JINGLE_RTP_H__

#include "jingleplugin.h"
#include "tag.h"

#include <list>
#include <string>
#include <vector>

namespace gloox {

namespace Jingle {
    /**
     * @brief a media type
     * https://www.iana.org/assignments/media-types/media-types.xhtml
     *
     */
    enum Media { invalid = -1, audio = 0, video = 1, application = 2 };
/**
 * @brief An abstraction of the signaling part of Jingle RTP Sessions
 * (@xep{0167}), implemented as a Jingle::Plugin.
 *
 * XEP Version: 0.15
 *
 * @author Jakob Schröter <js@camaya.net>
 * @since 1.0.7
 */
class GLOOX_API RTP : public Plugin {
public:

  typedef std::list<Media> Medias;

  struct Feedback {
    std::string type;
    std::string subtype;
  };
  typedef std::list<Feedback> Feedbacks;

  struct Parameter {
    std::string name;
    std::string value;
  };

  typedef std::list<Parameter> Parameters;

  /**
   * A struct holding information about a PayloadType.
   */
  struct PayloadType {
    int id;           /**< The type's id */
    std::string name; /**< The type's name. */
    int clockrate;   /**< The clockrate. */
    int bitrate;
    int channels;
    Parameters parameters;
    Feedbacks feedbacks;
  };

  /** A list of file information structs. */
  typedef std::list<PayloadType> PayloadTypes;

  struct HdrExt {
    int id;          /**< The type's id */
    std::string uri; /**< The type's name. */
  };
  typedef std::list<HdrExt> HdrExts;

  struct Source {
    std::string ssrc;
    std::string name;
    std::string videoType;
    std::string cname;
    std::string msid;
  };
  typedef std::vector<Source> Sources;

  struct SsrcGroup {
    std::string semantics;
    std::vector<std::string> ssrcs;
  };

  /**
   * Creates a new instance from the given Tag
   * @param tag The Tag to parse.
   */
  RTP(const Tag *tag = 0);

  RTP(Media media, const PayloadTypes &payloadTypes);

  /**
   * Virtual destructor.
   */
  virtual ~RTP() {}

  /**
   * Returns the type.
   * @return The type.
   */
  Media media() const;

  void setMedia(Media m);

  const PayloadTypes &payloadTypes() const;

  void setPayloadTypes(const PayloadTypes &);

  const HdrExts &hdrExts() const;
  void setHdrExts(const HdrExts &);

  const Sources &sources() const;
  void setSources(const Sources &);

  void setSsrcGroup(const SsrcGroup &);
  const SsrcGroup &ssrcGroup() const;

  bool rtcpMux() const;

  void setRtcpMux(bool rtcpMux);

  // reimplemented from Plugin
  virtual const StringList features() const;

  // reimplemented from Plugin
  virtual const std::string &filterString() const;

  // reimplemented from Plugin
  virtual Tag *tag() const;

  // reimplemented from Plugin
  virtual Plugin *newInstance(const Tag *tag) const;

  // reimplemented from Plugin
  virtual Plugin *clone() const { return new RTP(*this); }

private:
  void parsePayloadTypes(const TagList &payloadTypes);
  void addPayloadTypes(Tag* r) const;
  void parseHdrExts(const TagList &hdrExts);
  void addHdrExts(Tag* r) const;
  void parseSources(const TagList &tagList);
  void addSources(Tag* r) const;
  void parseSsrcGroups(const Tag *ssrcGroup);
  void addSsrcGroup(Tag* r) const;
  Parameters parseParameters(const TagList &tagList);

  PayloadTypes m_payloadTypes;
  HdrExts m_hdrExts;
  Sources m_sources;
  SsrcGroup m_ssrcGroup;
  Media m_media;
  bool m_rtcpMux;
};

} // namespace Jingle
} // namespace gloox

#endif // JINGLE_RTP_H__
