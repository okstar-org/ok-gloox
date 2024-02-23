/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL ) || defined( WANT_BOOKMARKSTORAGE )

#include "clientbase.h"
#include "pubsubitem.h"
#include "nativebookmarkstorage.h"
#include "nickname.h"

namespace gloox {

    NativeBookmarkStorage::NativeBookmarkStorage(ClientBase *parent)
        : m_bookmarkHandler(0), m_parent(parent) {
    }

    NativeBookmarkStorage::~NativeBookmarkStorage() {
    }

    void NativeBookmarkStorage::add(const BMConference &conference) {
      //item
      PubSub::Item *item = new PubSub::Item();

      //conference
      Tag *s = new Tag("conference", XMLNS, XMLNS_BOOKMARKS2);
      s->addAttribute("name", conference.name);
      s->addAttribute("autojoin", conference.autojoin);
      s->addChild(Nickname(conference.name).tag());

      //item.id
      item->setID(conference.jid);
      //item.conference
      item->setPayload(s);

      m_parent->send(item->tag());
    }

    void NativeBookmarkStorage::retrievesAll() {
      /**
       * https://xmpp.org/extensions/xep-0402.html#retrieving-bookmarks
       *
       * <iq from='juliet@capulet.lit/balcony' type='get' id='retrieve1'>
          <pubsub xmlns='http://jabber.org/protocol/pubsub'>
            <items node='urn:xmpp:bookmarks:1'/>
          </pubsub>
        </iq>
       */

      IQ iq(IQ::IqType::Get, JID(), m_parent->getID());
      iq.setFrom(JID(m_parent->username()+"@"+m_parent->server()));

      Tag* t = iq.tag();
      Tag *pubsub = new Tag(t, "pubsub", XMLNS, XMLNS_PUBSUB);
      new Tag(pubsub, "items", "node", XMLNS_BOOKMARKS2);

      m_parent->send(t);
    }

    bool NativeBookmarkStorage::handleIq(const gloox::IQ &iq) {

      BMConferenceList cList;

      Tag *xml = iq.tag();
      Tag *items = xml->findChild("items", "node", XMLNS_BOOKMARKS2);
      if (!items)
        return false;

      const TagList &l = items->findChildren("item");
      TagList::const_iterator it = l.begin();
      for (; it != l.end(); ++it) {
        BMConference conf;
        conf.jid = (*it)->findAttribute("id");
        Tag *cTag = (*it)->findChild("conference");
        if (!cTag)
          continue;
        /**
       * <conference xmlns='urn:xmpp:bookmarks:1'
                  name='The Play&apos;s the Thing'
                  autojoin='true'>
        <nick>JC</nick>
      </conference>
       */
        conf.name = cTag->findAttribute("name");
        conf.nick = cTag->findChild("nick")->cdata();
        conf.autojoin = cTag->findAttribute("autojoin") == "true";
        cList.push_back(conf);
      }

      if (m_bookmarkHandler)
        m_bookmarkHandler->handleBookmarks(cList);

      return true;
    }

    void NativeBookmarkStorage::handleIqID(const gloox::IQ &iq, int context) {

    }
}

#endif // GLOOX_MINIMAL
