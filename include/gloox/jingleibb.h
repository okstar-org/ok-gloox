/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef JINGLE_IBB_H__
#define JINGLE_IBB_H__

#include "jingleplugin.h"
#include "inbandbytestream.h"
#include <string>
#include <list>

namespace gloox {

    class Tag;

    namespace Jingle {

    class GLOOX_API IBB : public Plugin  {
        public:

            enum Action {
                init,
                open,
                data
            };

            struct Data {
                int seq;
                std::string chunk;
            };

            /**
             * Constructs a new instance.
             * @param pwd The @c pwd value.
             * @param ufrag The @c ufrag value.
             * @param candidates A list of connection candidates.
             */
            IBB(const std::string &sid, long blockSize);

            /**
             * Constructs a new instance from the given tag.
             * @param tag The Tag to parse.
             */
            IBB(const Tag *tag = 0);

            /**
             * Virtual destructor.
             */
            virtual ~IBB() {}

            const std::string &sid() const { return m_sid; }

            long blockSize() const { return m_blockSize; }

            Action action()const {return m_action;}

            const Data& getData() const {return m_data;}

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
                return new IBB(*this);
            }

        private:
            std::string m_sid;
            long m_blockSize;
            Action m_action;
            Data m_data;
        };
    }
}

#endif