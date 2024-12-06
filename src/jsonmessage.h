//
// Created by gaojie on 24-12-6.
//

#ifndef GLOOX_JSONMESSAGE_H
#define GLOOX_JSONMESSAGE_H

#include "gloox.h"
#include "stanzaextension.h"

namespace gloox {

class GLOOX_API JsonMessage : public StanzaExtension {
public:
  explicit JsonMessage();
  explicit JsonMessage(const Tag *tag);

  const std::string &filterString() const override;
  StanzaExtension *newInstance(const Tag *tag) const override;
  StanzaExtension *clone() const override;
  Tag *tag() const override;

  const std::string &getJson() const { return json; }

private:
  std::string json;
};
} // namespace gloox

#endif // JSONMESSAGE_H
