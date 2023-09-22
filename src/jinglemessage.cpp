/*
Copyright (c) 2007-2019 by Jakob Schröter <js@camaya.net>
This file is part of the gloox library. http://camaya.net/gloox

This software is distributed under a license. The full license
agreement can be found in the file LICENSE in this distribution.
This software may not be copied, modified, sold or distributed
other than expressed in the named license agreement.

This software is distributed without any warranty.
*/

#include "jinglemessage.h"
#include "tag.h"
#include "jinglertp.h"

namespace gloox {
namespace Jingle {

const std::string &JingleMessage::filterString() const {
  static const std::string filter //
      = "/message/propose[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
        "|/message/retract[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
        "|/message/accept[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
        "|/message/reject[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
        "|/message/finish[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
        "|/message/proceed[@xmlns='" + XMLNS_JINGLE_MESSAGE + "']" //
      ;
  return filter;
}

JingleMessage::JingleMessage(const Tag *tag)
    : StanzaExtension(ExtJingleMessage) {
  if (!tag)
    return;

  // id
  m_id = tag->findAttribute("id");
//  int event = util::lookup2(tag->name(), ActionValues) - 1;
//  if (event >= 0 && event < sizeof(ActionValues)) {
//    m_action = (Action)event;
//  }
    if(tag->name() == ActionValues[0]){
        m_action = propose;
    }else if(tag->name() == ActionValues[1]){
        m_action=retract;
    }else if(tag->name() == ActionValues[2]){
        m_action=accept;
    }else if(tag->name() == ActionValues[3]){
        m_action=reject;
    }else if(tag->name() == ActionValues[4]){
        m_action=proceed;
    }else if(tag->name() == ActionValues[5]){
        m_action=finish;
    }

  const TagList &descList = (tag)->findChildren("description");

  TagList ::const_iterator it2 = descList.begin();
  for (; it2 != descList.end(); ++it2) {
    const std::string& m = (*it2)->findAttribute("media");
    if (m == "audio") {
      m_medias.push_back(audio);
    } else if (m == "video") {
      m_medias.push_back(video);
    } else if (m == "application") {
      m_medias.push_back(application);
    }
  }
}


void JingleMessage::addMedia(Media media) {
    m_medias.push_back(media);
}

Tag *JingleMessage::tag() const {
  const char *v = ActionValues[m_action];
  Tag *x = new Tag(v, XMLNS, XMLNS_JINGLE_MESSAGE);
  x->addAttribute("id", m_id);

  RTP::Medias::const_iterator mit = m_medias.begin();
  for (; mit != m_medias.end(); ++mit) {
    Tag *desc = new Tag(x, "description", XMLNS, XMLNS_JINGLE_APPS_RTP);
    switch (*mit) {
    case audio:
      desc->addAttribute("media", "audio");
      break;
    case video:
      desc->addAttribute("media", "video");
      break;
    case application:
      desc->addAttribute("media", "application");
      break;
    }
  }
  return x;
}
} // namespace Jingle
} // namespace gloox