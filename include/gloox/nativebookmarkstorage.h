/**
* Copyright (c) 2022 船山科技 chuanshaninfo.com
OkEDU-Classroom is licensed under Mulan PubL v2.
You can use this software according to the terms and conditions of the Mulan
PubL v2. You may obtain a copy of Mulan PubL v2 at:
        http://license.coscl.org.cn/MulanPubL-2.0
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PubL v2 for more details.
*/

#if !defined( GLOOX_MINIMAL ) || defined( WANT_BOOKMARKSTORAGE )

#ifndef NATIVE_BOOKMARKSTORAGE_H__
#define NATIVE_BOOKMARKSTORAGE_H__

#include "macros.h"

#include "nativebookmarkhandler.h"
#include "pubsubmanager.h"

#include <string>
#include <list>

namespace gloox {

    class Tag;


    class GLOOX_API NativeBookmarkStorage : IqHandler {
    public:
        /**
         * Constructs a new NativeBookmarkStorage object.
         * @param parent The ClientBase to use for communication.
         */
        NativeBookmarkStorage(ClientBase *parent);

        /**
         * Virtual destructor.
         */
        virtual ~NativeBookmarkStorage();

        /**
         * retrieves all bookmarks
         * @link https://xmpp.org/extensions/xep-0402.html#retrieving-bookmarks
         */
        void retrievesAll();

        /**
         * add bookmark
         * https://xmpp.org/extensions/xep-0402.html#adding-a-bookmark
         */
        void add(const BMConference &conference);

        /**
         * Use this function to register a BookmarkHandler.
         * @param bmh The BookmarkHandler which shall receive retrieved bookmarks.
         */
        void registerBookmarkHandler(NativeBookmarkHandler *bmh) { m_bookmarkHandler = bmh; }

        /**
         * Use this function to un-register the BookmarkHandler.
         */
        void removeBookmarkHandler() { m_bookmarkHandler = 0; }

        virtual bool handleIq(const gloox::IQ &iq) override;

        virtual void handleIqID(const IQ &iq, int context) override;


    private:
        ClientBase *m_parent;
        NativeBookmarkHandler *m_bookmarkHandler;
    };

}

#endif // BOOKMARKSTORAGE_H__


#endif