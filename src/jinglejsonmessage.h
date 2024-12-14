/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef JINGLE_JSONMESSAGE_H__
#define JINGLE_JSONMESSAGE_H__

#include "jingleplugin.h"
#include "inbandbytestream.h"
#include <string>
#include <list>

namespace gloox {

    class Tag;

    namespace Jingle {

        class GLOOX_API JsonMessage : public Plugin {
        public:
            static const std::string TAG_NAME;

            /**
             * Constructs a new instance.
             * @param json The @c json value.
             */
            explicit JsonMessage(const std::string &json);

            /**
             * Constructs a new instance from the given tag.
             * @param tag The Tag to parse.
             */
            explicit JsonMessage(const Tag *tag = 0);

            /**
             * Virtual destructor.
             */
            virtual ~JsonMessage() {}

            const std::string &json() const { return m_json; }


            // reimplemented from Plugin
            virtual const StringList features() const;

            // reimplemented from Plugin
            virtual const std::string &filterString() const;

            // reimplemented from Plugin
            virtual Tag *tag() const;

            // reimplemented from Plugin
            virtual Plugin *newInstance(const Tag *tag) const;

            // reimplemented from Plugin
            virtual Plugin *clone() const {
                return new JsonMessage(*this);
            }

        private:
            std::string m_json;
        };
    }
}

#endif