/*
  Copyright (c) 2004-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#include "config.h"

#include "client.h"
#include "capabilities.h"
#include "rostermanager.h"
#include "disco.h"
#include "error.h"
#include "logsink.h"
#include "nonsaslauth.h"
#include "prep.h"
#include "stanzaextensionfactory.h"
#include "stanzaextension.h"
#include "tag.h"
#include "tlsbase.h"
#include "util.h"

#if !defined( _WIN32 ) && !defined( _WIN32_WCE )
# include <unistd.h>
#endif

namespace gloox
{

  // ---- Client::ResourceBind ----
  Client::ResourceBind::ResourceBind( const std::string& resource, bool bind )
    : StanzaExtension( ExtResourceBind ), m_jid( JID() ), m_bind( bind )
  {
    prep::resourceprep( resource, m_resource );
    m_valid = true;
  }

  Client::ResourceBind::ResourceBind( const Tag* tag )
    : StanzaExtension( ExtResourceBind ), m_resource( EmptyString ), m_bind( true )
  {
    if( !tag )
      return;

    if( tag->name() == "unbind" )
      m_bind = false;
    else if( tag->name() == "bind" )
      m_bind = true;
    else
      return;

    if( tag->hasChild( "jid" ) )
      m_jid.setJID( tag->findChild( "jid" )->cdata() );
    else if( tag->hasChild( "resource" ) )
      m_resource = tag->findChild( "resource" )->cdata();

    m_valid = true;
  }

  Client::ResourceBind::~ResourceBind()
  {
  }

  const std::string& Client::ResourceBind::filterString() const
  {
    static const std::string filter = "/iq/bind[@xmlns='" + XMLNS_STREAM_BIND + "']"
        "|/iq/unbind[@xmlns='" + XMLNS_STREAM_BIND + "']";
    return filter;
  }

  Tag* Client::ResourceBind::tag() const
  {
    if( !m_valid )
      return 0;

    Tag* t = new Tag( m_bind ? "bind" : "unbind" );
    t->setXmlns( XMLNS_STREAM_BIND );

    if( m_bind && m_resource.empty() && m_jid )
      new Tag( t, "jid", m_jid.full() );
    else
      new Tag( t, "resource", m_resource );

    return t;
  }
  // ---- ~Client::ResourceBind ----

  // ---- Client::SessionCreation ----
  Tag* Client::SessionCreation::tag() const
  {
    Tag* t = new Tag( "session" );
    t->setXmlns( XMLNS_STREAM_SESSION );
    return t;
  }
  // ---- Client::SessionCreation ----

  // ---- Client ----
  Client::Client( const std::string& server )
    : ClientBase( XMLNS_CLIENT, server ),
      m_rosterManager( 0 ), m_auth( 0 ),
      m_presence( Presence::Available, JID() ),
      m_forceNonSasl( false ), m_manageRoster( true ),
      m_smId( EmptyString ), m_smLocation( EmptyString ), m_smResume( false ), m_smWanted( false ), m_smMax( 0 ),
      m_streamFeatures( 0 )
  {
    m_jid.setServer( server );
    init();
  }

  Client::Client( const JID& jid, const std::string& password, int port )
    : ClientBase( XMLNS_CLIENT, password, EmptyString, port ),
      m_rosterManager( 0 ), m_auth( 0 ),
      m_presence( Presence::Available, JID() ),
      m_forceNonSasl( false ), m_manageRoster( true ),
      m_smId( EmptyString ), m_smLocation( EmptyString ), m_smResume( false ), m_smWanted( false ), m_smMax( 0 ),
      m_streamFeatures( 0 )
  {
    m_jid = jid;
    m_server = m_jid.serverRaw();
    init();
  }

  Client::~Client()
  {
    delete m_rosterManager;
    delete m_auth;
  }

  void Client::init()
  {
//    m_rosterManager = new RosterManager( this );
    m_disco->setIdentity( "client", "bot" );
    registerStanzaExtension( new ResourceBind( 0 ) );
    registerStanzaExtension( new Capabilities() );
    m_presenceExtensions.push_back( new Capabilities( m_disco ) );
  }

  void Client::setUsername( const std::string &username )
  {
    m_jid.setUsername( username );
  }

  bool Client::handleNormalNode( Tag* tag )
  {
    if( tag->name() == "features" && tag->xmlns() == XMLNS_STREAM )
    {
      m_streamFeatures = getStreamFeatures( tag );

      if( m_tls == TLSRequired && !m_encryptionActive
          && ( !m_encryption || !( m_streamFeatures & StreamFeatureStartTls ) ) )
      {
        logInstance().err( LogAreaClassClient, "Client is configured to require"
                                " TLS but either the server didn't offer TLS or"
                                " TLS support is not compiled into gloox." );
        disconnect( ConnTlsNotAvailable );
      }
      else if( m_tls > TLSDisabled && m_encryption && !m_encryptionActive
          && ( m_streamFeatures & StreamFeatureStartTls ) )
      {
        notifyStreamEvent( StreamEventEncryption );
        startTls();
      }
      else if( m_compress && m_compression && !m_compressionActive
          && ( m_streamFeatures & StreamFeatureCompressZlib ) )
      {
        notifyStreamEvent( StreamEventCompression );
        logInstance().warn( LogAreaClassClient, "The server offers compression, but negotiating Compression at this stage is not recommended. See XEP-0170 for details. We'll continue anyway." );
        negotiateCompression( StreamFeatureCompressZlib );
      }
      else if( m_sasl )
      {
        if( m_authed )
        {
          if( m_streamFeatures & StreamFeatureStreamManagement && m_smWanted && m_smContext >= CtxSMEnabled )
          {
            sendStreamManagement();
          }
          else if( m_streamFeatures & StreamFeatureBind && m_smContext < CtxSMEnabled )
          {
            notifyStreamEvent( StreamEventResourceBinding );
            bindResource( resource() );
          }
        }
        else if( !username().empty() && !password().empty() )
        {
          if( !login() )
          {
            logInstance().err( LogAreaClassClient, "The server doesn't support"
                                           " any auth mechanisms we know about" );
            disconnect( ConnNoSupportedAuth );
          }
        }
        else if( !m_clientCerts.empty() && !m_clientKey.empty()
                 && m_streamFeatures & SaslMechExternal && m_availableSaslMechs & SaslMechExternal )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechExternal );
        }
#if defined( _WIN32 ) && !defined( __SYMBIAN32__ )
        else if( m_streamFeatures & SaslMechGssapi && m_availableSaslMechs & SaslMechGssapi )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechGssapi );
        }
        else if( m_streamFeatures & SaslMechNTLM && m_availableSaslMechs & SaslMechNTLM )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechNTLM );
        }
#endif
        else if( m_streamFeatures & SaslMechAnonymous
                 && m_availableSaslMechs & SaslMechAnonymous )
        {
          notifyStreamEvent( StreamEventAuthentication );
          startSASL( SaslMechAnonymous );
        }
        else
        {
          notifyStreamEvent( StreamEventFinished );
          connected();
        }
      }
      else if( m_compress && m_compression && !m_compressionActive
          && ( m_streamFeatures & StreamFeatureCompressZlib ) )
      {
        notifyStreamEvent( StreamEventCompression );
        negotiateCompression( StreamFeatureCompressZlib );
      }
//       else if( ( m_streamFeatures & StreamFeatureCompressDclz )
//               && m_connection->initCompression( StreamFeatureCompressDclz ) )
//       {
//         negotiateCompression( StreamFeatureCompressDclz );
//       }
      else if( m_streamFeatures & StreamFeatureIqAuth )
      {
        notifyStreamEvent( StreamEventAuthentication );
        nonSaslLogin();
      }
      else
      {
        logInstance().err( LogAreaClassClient, "fallback: the server doesn't "
                                   "support any auth mechanisms we know about" );
        disconnect( ConnNoSupportedAuth );
      }
    }
    else
    {
      const std::string& name  = tag->name(),
                         xmlns = tag->findAttribute( XMLNS );
      if( name == "proceed" && xmlns == XMLNS_STREAM_TLS )
      {
        logInstance().dbg( LogAreaClassClient, "starting TLS handshake..." );

        if( m_encryption )
        {
          m_encryptionActive = true;
          m_encryption->handshake();
        }
      }
      else if( name == "failure" )
      {
        if( xmlns == XMLNS_STREAM_TLS )
        {
          logInstance().err( LogAreaClassClient, "TLS handshake failed (server-side)!" );
          disconnect( ConnTlsFailed );
        }
        else if( xmlns == XMLNS_COMPRESSION )
        {
          logInstance().err( LogAreaClassClient, "Stream compression init failed!" );
          disconnect( ConnCompressionFailed );
        }
        else if( xmlns == XMLNS_STREAM_SASL )
        {
          logInstance().err( LogAreaClassClient, "SASL authentication failed!" );
          processSASLError( tag );
          disconnect( ConnAuthenticationFailed );
        }
      }
      else if( name == "compressed" && xmlns == XMLNS_COMPRESSION )
      {
        logInstance().dbg( LogAreaClassClient, "Stream compression initialized" );
        m_compressionActive = true;
        header();
      }
      else if( name == "challenge" && xmlns == XMLNS_STREAM_SASL )
      {
        logInstance().dbg( LogAreaClassClient, "Processing SASL challenge" );
        processSASLChallenge( tag->cdata() );
      }
      else if( name == "success" && xmlns == XMLNS_STREAM_SASL )
      {
        if( !processSASLSuccess( tag->cdata() ) )
        {
          logInstance().err( LogAreaClassClient, "The Server response could not be verified!" );
          disconnect( ConnAuthenticationFailed );
          return false;
        }

        logInstance().dbg( LogAreaClassClient, "SASL authentication successful" );
        setAuthed( true );
        header();
      }
      else if( name == "enabled" && xmlns == XMLNS_STREAM_MANAGEMENT )
      {
        m_smContext = CtxSMEnabled;
        m_smMax = atoi( tag->findAttribute( "max" ).c_str() );
        m_smId = tag->findAttribute( "id" );
        const std::string res = tag->findAttribute( "resume" );
        m_smResume = ( ( res == "true" || res == "1" ) && !m_smId.empty() ) ? true : false;
        m_smLocation = tag->findAttribute( "location" );

        if( m_streamFeatures & StreamFeatureSession )
          createSession();
        else
          connected();
      }
      else if( name == "resumed" && xmlns == XMLNS_STREAM_MANAGEMENT && m_smContext == CtxSMResume )
      {
        if( tag->findAttribute( "previd" ) == m_smId )
        {
          m_smContext = CtxSMResumed;
          notifyStreamEvent( StreamEventSMResumed );
          int h = atoi( tag->findAttribute( "h" ).c_str() );
          connected();
          checkQueue( h, true );
        }
      }
      else if( name == "a" && xmlns == XMLNS_STREAM_MANAGEMENT && m_smContext >= CtxSMEnabled )
      {
        int h = atoi( tag->findAttribute( "h" ).c_str() );
        checkQueue( h, false );
      }
      else if( name == "r" && xmlns == XMLNS_STREAM_MANAGEMENT )
      {
        ackStreamManagement();
      }
      else if( name == "failed" && xmlns == XMLNS_STREAM_MANAGEMENT )
      {
        switch( m_smContext )
        {
          case CtxSMEnable:
            notifyStreamEvent( StreamEventSMEnableFailed );
            break;
          case CtxSMResume:
            notifyStreamEvent( StreamEventSMResumeFailed );
            break;
          default:
            break;
        }
        m_smContext = CtxSMFailed;
      }
      else
        return false;
    }

    return true;
  }

  int Client::getStreamFeatures( Tag* tag )
  {
    if( tag->name() != "features" || tag->xmlns() != XMLNS_STREAM )
      return 0;

    int features = 0;

    if( tag->hasChild( "starttls", XMLNS, XMLNS_STREAM_TLS ) )
      features |= StreamFeatureStartTls;

    if( tag->hasChild( "mechanisms", XMLNS, XMLNS_STREAM_SASL ) )
      features |= getSaslMechs( tag->findChild( "mechanisms" ) );

    if( tag->hasChild( "bind", XMLNS, XMLNS_STREAM_BIND ) )
      features |= StreamFeatureBind;

    if( tag->hasChild( "unbind", XMLNS, XMLNS_STREAM_BIND ) )
      features |= StreamFeatureUnbind;

    if( tag->hasChild( "session", XMLNS, XMLNS_STREAM_SESSION ) )
      features |= StreamFeatureSession;

    if( tag->hasChild( "auth", XMLNS, XMLNS_STREAM_IQAUTH ) )
      features |= StreamFeatureIqAuth;

    if( tag->hasChild( "register", XMLNS, XMLNS_STREAM_IQREGISTER ) )
      features |= StreamFeatureIqRegister;

    if( tag->hasChild( "compression", XMLNS, XMLNS_STREAM_COMPRESS ) )
      features |= getCompressionMethods( tag->findChild( "compression" ) );

    if( tag->hasChild( "sm", XMLNS, XMLNS_STREAM_MANAGEMENT ) )
      features |= StreamFeatureStreamManagement;

    if( tag->hasChild( "csi", XMLNS, XMLNS_CLIENT_STATE_INDICATION ) )
      features |= StreamFeatureClientStateIndication;

    if( features == 0 )
      features = StreamFeatureIqAuth;

    return features;
  }

  int Client::getSaslMechs( Tag* tag )
  {
    int mechs = SaslMechNone;

    const std::string mech = "mechanism";

    if( tag->hasChildWithCData( mech, "SCRAM-SHA-1-PLUS" ) )
      mechs |= SaslMechScramSha1Plus;

    if( tag->hasChildWithCData( mech, "SCRAM-SHA-1" ) )
      mechs |= SaslMechScramSha1;

    if( tag->hasChildWithCData( mech, "DIGEST-MD5" ) )
      mechs |= SaslMechDigestMd5;

    if( tag->hasChildWithCData( mech, "PLAIN" ) )
      mechs |= SaslMechPlain;

    if( tag->hasChildWithCData( mech, "ANONYMOUS" ) )
      mechs |= SaslMechAnonymous;

    if( tag->hasChildWithCData( mech, "EXTERNAL" ) )
      mechs |= SaslMechExternal;

    if( tag->hasChildWithCData( mech, "GSSAPI" ) )
      mechs |= SaslMechGssapi;

    if( tag->hasChildWithCData( mech, "NTLM" ) )
      mechs |= SaslMechNTLM;

    return mechs;
  }

  int Client::getCompressionMethods( Tag* tag )
  {
    int meths = 0;

    if( tag->hasChildWithCData( "method", "zlib" ) )
      meths |= StreamFeatureCompressZlib;

    if( tag->hasChildWithCData( "method", "lzw" ) )
      meths |= StreamFeatureCompressDclz;

    return meths;
  }

  bool Client::login()
  {
    bool retval = true;

    if( ( m_streamFeatures & SaslMechScramSha1Plus && m_availableSaslMechs & SaslMechScramSha1Plus
          && m_encryption && m_encryptionActive && m_encryption->hasChannelBinding() )
        && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechScramSha1Plus );
    }
    else if( m_streamFeatures & SaslMechScramSha1 && m_availableSaslMechs & SaslMechScramSha1
             && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechScramSha1 );
    }
    else if( m_streamFeatures & SaslMechDigestMd5 && m_availableSaslMechs & SaslMechDigestMd5
             && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechDigestMd5 );
    }
    else if( m_streamFeatures & SaslMechPlain && m_availableSaslMechs & SaslMechPlain
             && !m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      startSASL( SaslMechPlain );
    }
    else if( m_streamFeatures & StreamFeatureIqAuth || m_forceNonSasl )
    {
      notifyStreamEvent( StreamEventAuthentication );
      nonSaslLogin();
    }
    else
      retval = false;

    return retval;
  }

  void Client::handleIqIDForward( const IQ& iq, int context )
  {
    switch( context )
    {
      case CtxResourceUnbind:
        // we don't store known resources anyway
        break;
      case CtxResourceBind:
        processResourceBind( iq );
        break;
      case CtxSessionEstablishment:
        processCreateSession( iq );
        break;
      default:
        break;
    }
  }

  bool Client::bindOperation( const std::string& resource, bool bind )
  {
    if( !( m_streamFeatures & StreamFeatureUnbind ) && m_resourceBound )
      return false;

    IQ iq( IQ::Set, JID(), getID() );
    iq.addExtension( new ResourceBind( resource, bind ) );

    send( iq, this, bind ? CtxResourceBind : CtxResourceUnbind );
    return true;
  }

  bool Client::selectResource( const std::string& resource )
  {
    m_selectedResource = resource; // TODO: remove for 1.1
    m_jid.setResource( resource );

    if( !( m_streamFeatures & StreamFeatureUnbind ) )
      return false;

    return true;
  }

  void Client::processResourceBind( const IQ& iq )
  {
    switch( iq.subtype() )
    {
      case IQ::Result:
      {
        const ResourceBind* rb = iq.findExtension<ResourceBind>( ExtResourceBind );
        if( !rb || !rb->jid() )
        {
          notifyOnResourceBindError( 0 );
          break;
        }

        m_jid = rb->jid();
        m_resourceBound = true;
        m_selectedResource = m_jid.resource(); // TODO: remove for 1.1
        notifyOnResourceBind( m_jid.resource() );

        if( m_streamFeatures & StreamFeatureStreamManagement && m_smWanted )
          sendStreamManagement();
        else if( m_streamFeatures & StreamFeatureSession )
          createSession();
        else
          connected();
        break;
      }
      case IQ::Error:
      {
        notifyOnResourceBindError( iq.error() );
        break;
      }
      default:
        break;
    }
  }

  void Client::setStreamManagement( bool enable, bool resume )
  {
    m_smWanted = enable;
    m_smResume = resume;

    if( !m_smWanted )
    {
      m_smId = EmptyString;
      m_smLocation = EmptyString;
      m_smMax = 0;
      m_smResume = false;
      return;
    }

    if( m_smWanted && m_resourceBound )
      sendStreamManagement();
  }

  void Client::sendStreamManagement()
  {
    if( !m_smWanted )
      return;

    if( m_smContext == CtxSMInvalid )
    {
      notifyStreamEvent( StreamEventSMEnable );
      Tag* e = new Tag( "enable" );
      e->setXmlns( XMLNS_STREAM_MANAGEMENT );
      if( m_smResume )
        e->addAttribute( "resume", "true" );
      send( e );
      m_smContext = CtxSMEnable;
      m_smHandled = 0;
    }
    else if( m_smContext == CtxSMEnabled && m_smResume )
    {
      notifyStreamEvent( StreamEventSMResume );
      Tag* r = new Tag( "resume" );
      r->setXmlns( XMLNS_STREAM_MANAGEMENT );
      r->addAttribute( "h", m_smHandled );
      r->addAttribute( "previd", m_smId );
      send( r );
      m_smContext = CtxSMResume;
    }
    else
      disconnect();

  }

  void Client::ackStreamManagement()
  {
    if( m_smContext >= CtxSMEnabled )
    {
      Tag* a = new Tag( "a", "xmlns", XMLNS_STREAM_MANAGEMENT );
      a->addAttribute( "h", m_smHandled );
      send( a );
    }
  }

  void Client::reqStreamManagement()
  {
    if( m_smContext >= CtxSMEnabled )
    {
      Tag* r = new Tag( "r", "xmlns", XMLNS_STREAM_MANAGEMENT );
      send( r );
    }
  }

  void Client::createSession()
  {
    notifyStreamEvent( StreamEventSessionCreation );
    IQ iq( IQ::Set, JID(), getID() );
    iq.addExtension( new SessionCreation() );
    send( iq, this, CtxSessionEstablishment );
  }

  void Client::processCreateSession( const IQ& iq )
  {
    switch( iq.subtype() )
    {
      case IQ::Result:
        connected();
        break;
      case IQ::Error:
        notifyOnSessionCreateError( iq.error() );
        break;
      default:
        break;
    }
  }

  void Client::negotiateCompression( StreamFeature method )
  {
    Tag* t = new Tag( "compress", XMLNS, XMLNS_COMPRESSION );

    if( method == StreamFeatureCompressZlib )
      new Tag( t, "method", "zlib" );

    if( method == StreamFeatureCompressDclz )
      new Tag( t, "method", "lzw" );

    send( t );
  }

  void Client::setPresence( Presence::PresenceType pres, int priority,
                            const std::string& status )
  {
    m_presence.setPresence( pres );
    m_presence.setPriority( priority );
    m_presence.resetStatus();
    m_presence.addStatus( status );
    sendPresence( m_presence );
  }

  void Client::setPresence( const JID& to, Presence::PresenceType pres, int priority,
                            const std::string& status )
  {
    Presence p( pres, to, status, priority );
    sendPresence( p );
  }

  void Client::sendPresence( Presence& pres )
  {
    if( state() < StateConnected )
      return;

    send( pres );
  }

  void Client::disableRoster()
  {
    m_manageRoster = false;
    if(m_rosterManager){
      delete m_rosterManager;
      m_rosterManager = 0;
    }
  }

  RosterManager * Client::enableRoster() {
    if(m_rosterManager){
      return m_rosterManager;
    }

    m_rosterManager = new RosterManager(this);
    notifyStreamEvent( StreamEventRoster );
    m_rosterManager->fill();
    m_manageRoster = true;
    return m_rosterManager;
  }

#if !defined( GLOOX_MINIMAL ) || defined( WANT_NONSASLAUTH )
  void Client::nonSaslLogin()
  {
    if( !m_auth )
      m_auth = new NonSaslAuth( this );
    m_auth->doAuth( m_sid );
  }
#endif // GLOOX_MINIMAL

  void Client::connected()
  {
    if( m_authed && m_smContext != CtxSMResumed )
    {
        rosterFilled();
    }
    else
    {
      notifyStreamEvent( StreamEventFinished );
      notifyOnConnect();
    }
  }

  void Client::rosterFilled()
  {
    sendPresence( m_presence );
    notifyStreamEvent( StreamEventFinished );
    notifyOnConnect();
  }

  void Client::disconnect()
  {
    m_smContext = CtxSMInvalid;
    m_smHandled = 0;
    m_smId = EmptyString;
    m_smLocation = EmptyString;
    m_smMax = 0;
    m_smResume = false;
    m_smWanted = false;

    disconnect( ConnUserDisconnected );
  }

  void Client::disconnect( ConnectionError reason )
  {
    m_resourceBound = false;
    m_authed = false;
    m_streamFeatures = 0;
    ClientBase::disconnect( reason );
  }

  void Client::cleanup()
  {
    m_authed = false;
    m_resourceBound = false;
    m_streamFeatures = 0;
  }

}
