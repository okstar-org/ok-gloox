/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifndef GLOOX_JINGLEGROUP_H
#define GLOOX_JINGLEGROUP_H

#include "jingleplugin.h"

namespace gloox {

class Tag;

namespace Jingle {
/**
 * @brief An implementation of Jingle Grouping Framework (@xep{0338}) as a
 * StanzaExtension.
 *
 * @author Gao Jie <412555203@qq.com>
 * @since 1.0
 */
class Group : public Plugin {
public:
  //  enum Semantics { bundle, ls, semantics };
  typedef std::string Semantics;
  struct Content {
    std::string name;
  };
  typedef std::list<Content> ContentList;

  Group(Semantics semantics, const ContentList &contentList);
  Group(const Tag *tag = 0);

  Semantics getSemantics();

  ContentList contents();

  // reimplemented from Plugin
  virtual const StringList features() const;

  // reimplemented from Plugin
  virtual const std::string &filterString() const;

  // reimplemented from Plugin
  virtual Tag *tag() const;

  // reimplemented from Plugin
  virtual Plugin *newInstance(const Tag *tag) const;

  // reimplemented from Plugin
  virtual Plugin* clone() const
  {
    return new Group( *this );
  }

private:
  Semantics m_semantics;
  ContentList m_contents;
};

} // namespace Jingle
} // namespace gloox

#endif // GLOOX_JINGLEGROUP_H
