/*
Copyright (c) 2007-2019 by Jakob Schröter <js@camaya.net>
This file is part of the gloox library. http://camaya.net/gloox

This software is distributed under a license. The full license
agreement can be found in the file LICENSE in this distribution.
This software may not be copied, modified, sold or distributed
other than expressed in the named license agreement.

This software is distributed without any warranty.
*/

#ifndef GLOOX_JINGLEMESSAGE_H
#define GLOOX_JINGLEMESSAGE_H

#include "gloox.h"
#include "jinglertp.h"
#include "stanzaextension.h"

namespace gloox {
class Tag;
namespace Jingle {

static const char *ActionValues[] = {"propose", "retract", "accept",
                                     "reject",  "proceed", "finish"};

/**
 * @brief An implementation of Jingle Message Events (@xep{0353}) as a
 * StanzaExtension.
 *
 * @author Gao Jie <412555203@qq.com>
 * @since 1.0
 */
class GLOOX_API JingleMessage : public StanzaExtension {
public:
  enum Action { propose, retract, accept, reject, proceed, finish };

  /**
   * Constructs a new object from the given Tag.
   * @param tag A Tag to parse.
   */
  JingleMessage(const Tag *tag = NULL);

  JingleMessage(Action action, const std::string &id = EmptyString)
      : StanzaExtension(ExtJingleMessage), m_action(action), m_id(id) {}

  /**
   * Virtual destructor.
   */
  virtual ~JingleMessage() {}

  // reimplemented from StanzaExtension
  virtual const std::string &filterString() const;

  // reimplemented from StanzaExtension
  virtual StanzaExtension *newInstance(const Tag *tag) const {
    return new JingleMessage(tag);
  }

  // reimplemented from StanzaExtension
  virtual Tag *tag() const;

  // reimplemented from StanzaExtension
  StanzaExtension *clone() const { return new JingleMessage(*this); }

  Action action() const { return m_action; }

  const std::string &id() const { return m_id; }

  const RTP::Medias &medias() const { return m_medias; }

  void addMedia(Media media);

private:
  Action m_action;
  std::string m_id;
  RTP::Medias m_medias;
};

} // namespace Jingle
} // namespace gloox

#endif // GLOOX_JINGLEMESSAGE_H
