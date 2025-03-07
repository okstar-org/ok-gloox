/*
  Copyright (c) 2013-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#if !defined( GLOOX_MINIMAL ) || defined( WANT_JINGLE )

#include "jinglepluginfactory.h"
#include "tag.h"
#include "util.h"

namespace gloox
{

  namespace Jingle
  {

    PluginFactory::PluginFactory()
    {
    }

    PluginFactory::~PluginFactory()
    {
      util::clearList( m_plugins );
    }

    void PluginFactory::registerPlugin( Plugin* plugin )
    {
      if( !plugin )
        return;

      plugin->setFactory( this );
      m_plugins.push_back( plugin );
    }

    void PluginFactory::addPlugins( Plugin& plugin, const Tag* tag )
    {
      if( !tag )
        return;

      

      PluginList::const_iterator itp = m_plugins.begin();
      for( ; itp != m_plugins.end(); ++itp )
      {
        const ConstTagList& match = tag->findTagList( (*itp)->filterString() );
        ConstTagList::const_iterator it;
        for(it = match.begin(); it != match.end(); ++it )
        {
          Plugin* pl = (*itp)->newInstance( (*it) );
          if( pl )
          {
            plugin.addPlugin( pl );
          }
        }
      }
    }

    void PluginFactory::addPlugins( Session::Jingle& jingle, const Tag* tag )
    {
      if( !tag )
        return;

      ConstTagList::const_iterator it;

      PluginList::const_iterator itp = m_plugins.begin();
      for( ; itp != m_plugins.end(); ++itp )
      {
        const ConstTagList& match = tag->findTagList((*itp)->filterString());
        it = match.begin();
        for( ; it != match.end(); ++it )
        {
          Plugin* pl = (*itp)->newInstance( (*it) );
          if( pl )
          {
            jingle.addPlugin( pl );
          }
        }
      }
    }


  }

}

#endif // GLOOX_MINIMAL
