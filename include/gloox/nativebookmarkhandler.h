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


#ifndef NATIVE_BOOKMARKHANDLER_H__
#define NATIVE_BOOKMARKHANDLER_H__

#include "macros.h"

#include <string>
#include <list>

namespace gloox
{

  struct BMConference
  {
    std::string name;  /**< A human readable name of the conference room. */
    std::string jid;   /**< The address of the room. */
    std::string nick;  /**< The nick name to use in this room. */
    bool autojoin;     /**< The conference shall be joined automatically on login. */
  };

  /**
   * A list of conference items.
   */
  typedef std::list<BMConference> BMConferenceList;

  /**
   * @brief A virtual interface which can be reimplemented to receive bookmarks with help of a
   * BookmarkStorage object.
   *
   * @author Jakob Schröter <js@camaya.net>
   * @since 0.3
   */
  class GLOOX_API NativeBookmarkHandler
  {
    public:
      /**
       * Virtual Destructor.
       */
      virtual ~NativeBookmarkHandler() {}

      /**
       * This function is called when bookmarks arrive from the server.
       * @param bList A list of URL bookmarks.
       * @param cList A list of conference bookmarks.
       */
      virtual void handleBookmarks(const BMConferenceList &cList ) = 0;
  };

}

#endif // BOOKMARKHANDLER_H__

#endif