#ifndef GLOOX_AVATAR_H
#define GLOOX_AVATAR_H

#include <string>
#include "gloox.h"
#include "tag.h"
#include "stanzaextension.h"
#include <list>
#include <map>

namespace gloox {

    class GLOOX_API AvatarMeta : public StanzaExtension {
    public:
        AvatarMeta(const AvatarMeta &data);

        AvatarMeta(long bytes,
                   int height,
                   int width,
                   const std::string &type,
                   const std::string &id);

        AvatarMeta(const Tag *tag = 0);

        virtual ~AvatarMeta() {}

        // reimplemented from StanzaExtension
        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new AvatarMeta(tag);
        }

        virtual StanzaExtension *clone() const {
            return new AvatarMeta(*this);
        }

        virtual Tag *tag() const;

        const long &bytes() const {
            return m_bytes;
        }

        const int width() const {
            return m_width;
        }

        const int height() const {
            return m_height;
        }

        const std::string &type() const {
            return m_type;
        }

        const std::string &id() const {
            return m_id;
        }

    private:
        long m_bytes;
        int m_height;
        int m_width;
        std::string m_type;
        std::string m_id;
    };

    class GLOOX_API AvatarData : public StanzaExtension {
    public:
        AvatarData(const AvatarData &data);

        AvatarData(const std::string &data);

        AvatarData(const Tag *tag = 0);

        virtual ~AvatarData() {}

        // reimplemented from StanzaExtension
        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new AvatarData(tag);
        }

        virtual StanzaExtension *clone() const {
            return new AvatarData(*this);
        }

        virtual Tag *tag() const;

        const std::string &data() const {
            return m_data;
        }

    private:
        std::string m_data;
    };


    class GLOOX_API Avatar : public StanzaExtension {
    public:

        Avatar(const Avatar &avatar);

        Avatar(const Tag *tag = 0);

        Avatar(const std::list<std::string> &ids);

        ~Avatar();

        // reimplemented from StanzaExtension
        virtual const std::string &filterString() const;

        virtual StanzaExtension *newInstance(const Tag *tag) const {
            return new Avatar(tag);
        }

        virtual StanzaExtension *clone() const {
            return new Avatar(*this);
        }

        virtual Tag *tag() const;

        // reimplemented from StanzaExtension

        const std::map<std::string, std::string> &data() const {
            return m_datas;
        }

    private:

        /**
         * SHA-1 hash of the data for the "image/png" content-type
         */
        std::map<std::string, std::string> m_datas;

        std::list<std::string> m_itemIds;
    };
}
#endif //GLOOX_AVATAR_H
