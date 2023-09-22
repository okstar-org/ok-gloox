/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "avatar.h"
#include <stdlib.h>


namespace gloox {

    AvatarMeta::AvatarMeta(const AvatarMeta &data) :
            StanzaExtension(ExtAvatarMeta),
            m_bytes(data.m_bytes),
            m_height(data.m_height),
            m_width(data.m_width),
            m_type(data.m_type),
            m_id(data.m_id)
            {
    }

    AvatarMeta::AvatarMeta(long bytes,
                           int height,
                           int width,
                           const std::string &type,
                           const std::string &id) :
            StanzaExtension(ExtAvatarMeta),
            m_bytes(bytes),
            m_height(height),
            m_width(width),
            m_type(type),
            m_id(id)
    {
    }

    AvatarMeta::AvatarMeta(const Tag *tag) :
            StanzaExtension(ExtAvatarMeta) {
        if (!tag) {
            return;
        }

        Tag* data = tag->findChild("metadata", "xmlns", XMLNS_META_AVATAR);
        if (!data) {
            return;
        }
        Tag* info = data->findChild("info");
        if(!info){
            return;
        }

        m_bytes = std::strtol(info->findAttribute("bytes").data(), NULL, 0);
        m_id = info->findAttribute("id");
        m_type = info->findAttribute("type");
        m_height = std::atoi( info->findAttribute("height").data() );
        m_width = std::atoi( info->findAttribute("width").data() );
    }

    const std::string &AvatarMeta::filterString() const {
        static const std::string filter =
                "metadata[@xmlns='" + XMLNS_AVATAR + "']";
        return filter;
    }

    Tag *AvatarMeta::tag() const {
        /**
        * <metadata xmlns='urn:xmpp:avatar:metadata'>
         <info bytes='12345'
               id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'
               height='64'
               type='image/png'
               width='64'/>
       </metadata>
        */
        Tag *t = new Tag("metadata", "xmlns", XMLNS_META_AVATAR);
        Tag* info = new Tag(t, "info");
        info->addAttribute("id", m_id);
        info->addAttribute("bytes", m_bytes);
        info->addAttribute("height", m_height);
        info->addAttribute("width", m_width);
        info->addAttribute("type", m_type);
        return t;
    }


    AvatarData::AvatarData(const AvatarData &data) :
            StanzaExtension(ExtAvatarData), m_data(data.data()) {

    }

    AvatarData::AvatarData(const std::string &data) :
            StanzaExtension(ExtAvatarData), m_data(data) {
    }

    AvatarData::AvatarData(const Tag *tag) :
            StanzaExtension(ExtAvatarData) {
        if (!tag) {
            return;
        }
        /**
                <data xmlns='urn:xmpp:avatar:data'>
                  qANQR1DBwU4DX7jmYZnncm...
                </data>
         */
        Tag* data = tag->findChild("data", "xmlns", XMLNS_AVATAR);
        if (data) {
            m_data = data->cdata();
        }

    }

    const std::string &AvatarData::filterString() const {
        static const std::string filter =
                "/iq/pubsub/publish[@node='" + XMLNS_AVATAR + "']/item/data[@node='" + XMLNS_AVATAR + "']";
        return filter;
    }

    Tag *AvatarData::tag() const {
        /**
         * <data xmlns='urn:xmpp:avatar:data'>
                  qANQR1DBwU4DX7jmYZnncm...
                </data>
         */
        Tag *t = new Tag("data", "xmlns", XMLNS_AVATAR);
        t->setCData(m_data);
        return t;
    }

    // =============Avatar===============//

    Avatar::Avatar(const Avatar &avatar) :
            StanzaExtension(ExtAvatar), m_datas(avatar.m_datas) {
    }

    Avatar::Avatar(const Tag *tag) : StanzaExtension(ExtAvatar) {
        if (!tag) {
            return;
        }
        /**
         *  <items node='urn:xmpp:avatar:data'>
              <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'>
                <data xmlns='urn:xmpp:avatar:data'>
                  qANQR1DBwU4DX7jmYZnncm...
                </data>
              </item>
            </items>
         */
        TagList itemList = tag->findChildren("item");
        TagList::const_iterator it = itemList.begin();
        for (; it != itemList.end(); ++it) {
            std::string id = (*it)->findAttribute("id");
            Tag *data = (*it)->findChild("data", XMLNS, XMLNS_AVATAR);
            if (data) {
                m_datas.insert(std::make_pair(id, data->cdata()));
            }
        }
    }

    Avatar::Avatar(const std::list<std::string> &ids) : StanzaExtension(ExtAvatar),
                                                        m_itemIds(ids) {
    }

    Avatar::~Avatar() {
    }

    const std::string &Avatar::filterString() const {
        static const std::string filter = "/iq/pubsub/items[@node='" + XMLNS_AVATAR + "']";
        return filter;
    }

    Tag *Avatar::tag() const {
        /**
         * <items node='urn:xmpp:avatar:data'>
            <item id='111f4b3c50d7b0df729d299bc6f8e9ef9066971f'/>
           </items>
         */
        Tag *t = new Tag("items", "node", XMLNS_AVATAR);
        std::list<std::string>::const_iterator it = m_itemIds.begin();
        for (; it != m_itemIds.end(); ++it) {
            Tag *item = new Tag(t, "item");
            item->addAttribute("id", *it);
        }
        return t;
    }

}
