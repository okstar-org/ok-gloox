//
// Created by gaojie on 24-12-6.
//
#include "jsonmessage.h"
#include "tag.h"

namespace gloox {

JsonMessage::JsonMessage(): StanzaExtension(ExtMeetJsonMessage) {

}

JsonMessage::JsonMessage(const gloox::Tag *tag)
    : StanzaExtension(ExtMeetJsonMessage) {
  if (!tag)
    return;

  json = tag->cdata();
}

Tag* JsonMessage::tag() const {
  Tag *t = new Tag("json-message", "xmlns", XMLNS_JIT_MEET);
  t->setCData(json);
  return t;
}

const std::string &JsonMessage::filterString() const {
  static const std::string filter //
      = "/message/json-message[@xmlns='" + XMLNS_JIT_MEET + "']";
  return filter;
}

StanzaExtension *JsonMessage::newInstance(const Tag *tag) const {
  return new JsonMessage(tag);
}

StanzaExtension *JsonMessage::clone() const { return new JsonMessage(*this); }
} // namespace gloox