/*
  Copyright (c) 2009-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsopensslbase.h"

#ifdef HAVE_OPENSSL

#include <algorithm>
#include <cctype>
#include <ctime>
#include <cstdlib>
#include <iostream>

#include <string.h>

#include <openssl/err.h>
#ifndef _WIN32
#include <strings.h>
#else // _WIN32
#if defined( _WIN32 ) || defined( _WIN64 )
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#endif // _WIN32 || _WIN64
#endif
#include <openssl/x509v3.h>
#include <openssl/ssl.h>

#define HOSTNAME_MAX_SIZE 255

namespace gloox
{
#ifdef HAVE_PTHREAD
  pthread_mutex_t **OpenSSLBase::lockarray = 0;
#endif // HAVE_PTHREAD

  OpenSSLBase::OpenSSLBase( TLSHandler* th, const std::string& server )
    : TLSBase( th, server ), m_ssl( 0 ), m_ctx( 0 ), m_buf( 0 ), m_bufsize( 17000 )
  {
    m_buf = static_cast<char*>( calloc( m_bufsize + 1, sizeof( char ) ) );
#ifdef HAVE_PTHREAD
    initLocks();
#endif // HAVE_PTHREAD
  }

  OpenSSLBase::~OpenSSLBase()
  {
#ifdef HAVE_PTHREAD
    killLocks();
#endif // HAVE_PTHREAD
    m_handler = 0;
    free( m_buf );
    SSL_CTX_free( m_ctx );
    SSL_shutdown( m_ssl );
    SSL_free( m_ssl );
    BIO_free( m_nbio );
    cleanup();
  }

#ifdef HAVE_PTHREAD
  void OpenSSLBase::lockCallback( int mode, int type, char *file, int line )
  {
    //std::cerr << "OpenSSLBase::lock_callback : "  << file  << "line " << line << std::endl;
    if( !OpenSSLBase::lockarray[type] )
      return;

    if( mode & CRYPTO_LOCK )
    {
      pthread_mutex_lock( OpenSSLBase::lockarray[type] );
    }
    else
    {
      pthread_mutex_unlock( OpenSSLBase::lockarray[type] );
    }
  }
  
  unsigned long OpenSSLBase::threadId()
  {
    unsigned long ret;
   
    ret = static_cast<unsigned long>( pthread_self() );
    //std::cerr << "OpenSSLBase::thread_id : " << ret << std::endl;
    return ret;
  }

  void OpenSSLBase::initLocks()
  {
    int i;
    static bool init_done = false;

    //std::cerr << "OpenSSLBase::init_locks "  << std::endl;
    if( init_done == false )
    {
      OpenSSLBase::lockarray = static_cast<pthread_mutex_t **>( OPENSSL_malloc( CRYPTO_num_locks() *
                                                                sizeof( pthread_mutex_t *) ) );
      for( i = 0; i < CRYPTO_num_locks(); i++ )
      {
        OpenSSLBase::lockarray[i] = static_cast<pthread_mutex_t *>( OPENSSL_malloc( sizeof( pthread_mutex_t ) ) );
        pthread_mutex_init( OpenSSLBase::lockarray[i], 0 );
        //std::cerr << "OpenSSLBase::init_locks OK "  << i << std::endl;
      }
      
      CRYPTO_set_id_callback( (unsigned long (*)())threadId );
      CRYPTO_set_locking_callback( (void (*)(int, int, const char*, int) )lockCallback );
      init_done = true;
    }
  }

  void OpenSSLBase::killLocks()
  {
    if( OpenSSLBase::lockarray == 0 )
      return;

    int i;
    
    //std::cerr << "OpenSSLBase::kill_locks "  << std::endl;
    CRYPTO_set_locking_callback( 0 );
    for( i = 0; i < CRYPTO_num_locks(); i++ )
    {
      //std::cerr << "OpenSSLBase::kill_locks "  << i << std::endl;
      if( OpenSSLBase::lockarray && OpenSSLBase::lockarray[i] )
      {
        pthread_mutex_destroy( OpenSSLBase::lockarray[i] );
        OPENSSL_free( OpenSSLBase::lockarray[i] );
        OpenSSLBase::lockarray[i] = 0;
        //std::cerr << "OpenSSLBase::kill_locks OK "  << i << std::endl;
      }
    }
    
    if( OpenSSLBase::lockarray )
    {
      OPENSSL_free( OpenSSLBase::lockarray );
      OpenSSLBase::lockarray = 0;
    }
  }
#endif // HAVE_PTHREAD

  bool OpenSSLBase::init( const std::string& clientKey,
                          const std::string& clientCerts,
                          const StringList& cacerts )
  {
#if defined OPENSSL_VERSION_NUMBER && ( OPENSSL_VERSION_NUMBER < 0x10100000 )
    if( m_initLib )
      SSL_library_init();
#endif // OPENSSL_VERSION_NUMBER < 0x10100000

    SSL_COMP_add_compression_method( 193, COMP_zlib() );

    OpenSSL_add_all_algorithms();

    if( !createCTX() )
      return false;

    setClientCert( clientKey, clientCerts );
    setCACerts( cacerts );

    if( !SSL_CTX_set_cipher_list( m_ctx, "HIGH:MEDIUM:AES:@STRENGTH" ) )
      return false;

    m_ssl = SSL_new( m_ctx );
    if( !m_ssl )
      return false;

    SSL_CTX_set_verify( m_ctx, SSL_VERIFY_PEER, 0 );

    if( !BIO_new_bio_pair( &m_ibio, 0, &m_nbio, 0 ) )
      return false;

    SSL_set_bio( m_ssl, m_ibio, m_ibio );
    SSL_set_mode( m_ssl, SSL_MODE_AUTO_RETRY | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE );

    ERR_load_crypto_strings();
    SSL_load_error_strings();

    if( !privateInit() )
      return false;

    m_valid = true;
    return true;
  }

  bool OpenSSLBase::encrypt( const std::string& data )
  {
    m_sendBuffer += data;

    if( !m_secure )
    {
      handshake();
      return 0;
    }

    doTLSOperation( TLSWrite );
    return true;
  }

  int OpenSSLBase::decrypt( const std::string& data )
  {
    m_recvBuffer += data;

    if( !m_secure )
    {
      handshake();
      return 0;
    }

    doTLSOperation( TLSRead );
    return true;
  }

  void OpenSSLBase::setCACerts( const StringList& cacerts )
  {
    m_cacerts = cacerts;

    if( m_cacerts.empty() )
      SSL_CTX_set_default_verify_paths( m_ctx );

    StringList::const_iterator it = m_cacerts.begin();
    for( ; it != m_cacerts.end(); ++it )
      SSL_CTX_load_verify_locations( m_ctx, (*it).c_str(), 0 );
  }

  void OpenSSLBase::setClientCert( const std::string& clientKey, const std::string& clientCerts )
  {
    m_clientKey = clientKey;
    m_clientCerts = clientCerts;

    if( !m_clientKey.empty() && !m_clientCerts.empty() )
    {
      if( SSL_CTX_use_certificate_chain_file( m_ctx, m_clientCerts.c_str() ) != 1 )
      {
        // FIXME
      }
      if( SSL_CTX_use_RSAPrivateKey_file( m_ctx, m_clientKey.c_str(), SSL_FILETYPE_PEM ) != 1 )
      {
        // FIXME
      }
    }

    if ( SSL_CTX_check_private_key( m_ctx ) != 1 )
    {
        // FIXME
    }
  }

  void OpenSSLBase::cleanup()
  {
    if( !m_mutex.trylock() )
      return;

    m_secure = false;
    m_valid = false;

    m_mutex.unlock();
  }

  void OpenSSLBase::doTLSOperation( TLSOperation op )
  {
    if( !m_handler )
      return;

    int ret = 0;
    bool onceAgain = false;

    do
    {
      switch( op )
      {
        case TLSHandshake:
          ret = handshakeFunction();
          break;
        case TLSWrite:
          ret = SSL_write( m_ssl, m_sendBuffer.c_str(), static_cast<int>( m_sendBuffer.length() ) );
          break;
        case TLSRead:
          m_buf[0] = static_cast<char>( 0xAA );
          m_buf[1] = static_cast<char>( 0xAA );
          m_buf[2] = static_cast<char>( 0xAA );
          ret = SSL_read( m_ssl, m_buf, m_bufsize );
          break;
      }

      switch( SSL_get_error( m_ssl, ret ) )
      {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          pushFunc();
          break;
        case SSL_ERROR_NONE:
          if( op == TLSHandshake )
            m_secure = true;
          else if( op == TLSWrite )
            m_sendBuffer.erase( 0, ret );
          else if( op == TLSRead )
            m_handler->handleDecryptedData( this, std::string( m_buf, ret ) );
          pushFunc();
          break;
        case SSL_ERROR_SSL:
//           std::cerr << "OpenSSLBase::doTLSOperation SSL_ERROR_SSL occured"<< std::endl;
          break;
        case SSL_ERROR_WANT_X509_LOOKUP:
//           std::cerr << "OpenSSLBase::doTLSOperation SSL_ERROR_WANT_X509_LOOKUP occured"<< std::endl;
          break;
        case SSL_ERROR_SYSCALL:
          /**
           * look at error stack/returnued errno
           */
          //std::cerr << "OpenSSLBase::doTLSOperation SSL_ERROR_SYSCALL occured : "<< ERR_get_error() << std::endl;
          ERR_print_errors_fp( stderr );
          ERR_clear_error();
          break;
        case SSL_ERROR_ZERO_RETURN:
//           std::cerr << "OpenSSLBase::doTLSOperation SSL_ERROR_ZERO_RETURN occured : "<< std::endl;
          break;
        case SSL_ERROR_WANT_CONNECT:
        case SSL_ERROR_WANT_ACCEPT:
        default:
          if( !m_secure )
            m_handler->handleHandshakeResult( this, false, m_certInfo );
          return;
          break;
      }
      if( !onceAgain && !m_recvBuffer.length() )
        onceAgain = true;
      else if( onceAgain )
        onceAgain = false;
    }
    while( ( ( onceAgain || m_recvBuffer.length() ) && ( !m_secure || op == TLSRead ) )
           || ( ( op == TLSWrite ) && ( ret > 0 ) ));
  }

  int OpenSSLBase::ASN1Time2UnixTime( ASN1_TIME* time )
  {
    struct tm t;
    const char* str = reinterpret_cast<const char*>( time->data );
    size_t i = 0;

    memset( &t, 0, sizeof(t) );

    if( time->type == V_ASN1_UTCTIME ) /* two digit year */
    {
      t.tm_year = ( str[i++] - '0' ) * 10;
      t.tm_year += ( str[i++] - '0' );

      if( t.tm_year < 70 )
        t.tm_year += 100;
    }
    else if( time->type == V_ASN1_GENERALIZEDTIME ) /* four digit year */
    {
      t.tm_year = ( str[i++] - '0' ) * 1000;
      t.tm_year += ( str[i++] - '0' ) * 100;
      t.tm_year += ( str[i++] - '0' ) * 10;
      t.tm_year += ( str[i++] - '0' );
      t.tm_year -= 1900;
    }

    t.tm_mon = ( str[i++] - '0' ) * 10;
    t.tm_mon += ( str[i++] - '0' ) - 1; // -1 since January is 0 not 1.
    t.tm_mday = ( str[i++] - '0' ) * 10;
    t.tm_mday += ( str[i++] - '0' );
    t.tm_hour = ( str[i++] - '0' ) * 10;
    t.tm_hour += ( str[i++] - '0' );
    t.tm_min = ( str[i++] - '0' ) * 10;
    t.tm_min += ( str[i++] - '0' );
    t.tm_sec = ( str[i++] - '0' ) * 10;
    t.tm_sec += ( str[i++] - '0' );

    return static_cast<int>( mktime( &t ) );
  }

#if defined OPENSSL_VERSION_NUMBER && ( OPENSSL_VERSION_NUMBER < 0x10100000 )
  int SSL_SESSION_get_protocol_version( const SSL_SESSION* s )
  {
    return s->ssl_version;
  }
#endif // OPENSSL_VERSION_NUMBER < 0x10100000

  bool OpenSSLBase::handshake()
  {

    doTLSOperation( TLSHandshake );

    if( !m_secure )
      return true;

    long res = SSL_get_verify_result( m_ssl );
    if( res != X509_V_OK )
      m_certInfo.status = CertInvalid;
    else
      m_certInfo.status = CertOk;

    X509* peer = SSL_get_peer_certificate( m_ssl );
    if( peer )
    {
      char peer_CN[256];
      X509_NAME_get_text_by_NID( X509_get_issuer_name( peer ), NID_commonName, peer_CN, sizeof( peer_CN ) );
      m_certInfo.issuer = peer_CN;
      X509_NAME_get_text_by_NID( X509_get_subject_name( peer ), NID_commonName, peer_CN, sizeof( peer_CN ) );
      m_certInfo.server = peer_CN;
      m_certInfo.date_from = ASN1Time2UnixTime( X509_get_notBefore( peer ) );
      m_certInfo.date_to = ASN1Time2UnixTime( X509_get_notAfter( peer ) );
      std::string p( peer_CN );
      std::transform( p.begin(), p.end(), p.begin(), tolower );

#if defined OPENSSL_VERSION_NUMBER && ( OPENSSL_VERSION_NUMBER >= 0x10002000 )
      res = X509_check_host( peer, p.c_str(), p.length(), X509_CHECK_FLAG_MULTI_LABEL_WILDCARDS, 0 );
      if( res <= 0 ) // 0: verification failed; -1: internal error; -2 input is malformed
        m_certInfo.status |= CertWrongPeer;
#else
      if( validateHostname( m_server.c_str(), peer, m_certInfo.listSAN ) != OpenSSLBase::MatchFound )
        m_certInfo.status |= CertWrongPeer;
#endif // OPENSSL_VERSION_NUMBER >= 0x10002000

      if( ASN1_UTCTIME_cmp_time_t( X509_get_notBefore( peer ), time( 0 ) ) != -1 )
        m_certInfo.status |= CertNotActive;

      if( ASN1_UTCTIME_cmp_time_t( X509_get_notAfter( peer ), time( 0 ) ) != 1 )
        m_certInfo.status |= CertExpired;

      X509_free( peer );
    }
    else
    {
      m_certInfo.status = CertInvalid;
    }

    const char* tmp;
    tmp = SSL_get_cipher_name( m_ssl );
    if( tmp )
      m_certInfo.cipher = tmp;

    SSL_SESSION* sess = SSL_get_session( m_ssl );
    if( sess )
    {
      switch( SSL_SESSION_get_protocol_version( sess ) )
      {
        case TLS1_VERSION:
          m_certInfo.protocol = TLSv1;
          break;
        case TLS1_1_VERSION:
          m_certInfo.protocol = TLSv1_1;
          break;
        case TLS1_2_VERSION:
          m_certInfo.protocol = TLSv1_2;
          break;
        case TLS1_3_VERSION:
          m_certInfo.protocol = TLSv1_3;
          break;
        case DTLS1_VERSION:
          m_certInfo.protocol = DTLSv1;
          break;
        case DTLS1_2_VERSION:
          m_certInfo.protocol = DTLSv1_2;
          break;
        default:
          m_certInfo.protocol = TLSInvalid;
          break;
      }
    }

    tmp = SSL_COMP_get_name( SSL_get_current_compression( m_ssl ) );
    if( tmp )
      m_certInfo.compression = tmp;

    m_valid = true;

    m_handler->handleHandshakeResult( this, true, m_certInfo );
    return true;
  }

  void OpenSSLBase::pushFunc()
  {
    int wantwrite;
    size_t wantread;
    long frombio;
    long tobio;

    while( ( wantwrite = BIO_pending( m_nbio ) ) > 0 )
    {
      if( wantwrite > m_bufsize )
        wantwrite = m_bufsize;

      if( !wantwrite )
        break;

      frombio = BIO_read( m_nbio, m_buf, wantwrite );

      if( frombio > 0 && m_handler )
        m_handler->handleEncryptedData( this, std::string( m_buf, frombio ) );
    }

    while( ( wantread = BIO_ctrl_get_read_request( m_nbio ) ) > 0 )
    {
      if( wantread > m_recvBuffer.length() )
        wantread = m_recvBuffer.length();

      if( !wantread )
        break;

      tobio = BIO_write( m_nbio, m_recvBuffer.c_str(), static_cast<int>( wantread ) );
      m_recvBuffer.erase( 0, tobio );
    }
  }

  void OpenSSLBase::fillSubjectAlternativeName( StringList& list, const X509 *serverCert )
  {
    int i;
    int sanNamesNb = -1;
    STACK_OF( GENERAL_NAME ) *sanNames = 0;
    
    // Try to extract the names within the SAN extension from the certificate
    sanNames = static_cast<stack_st_GENERAL_NAME *>( X509_get_ext_d2i(
                                                                static_cast<const X509*>( serverCert ),
                                                                NID_subject_alt_name, 0, 0 )
                                                    );
    if( sanNames == 0 )
    {
      return;
    }
    sanNamesNb = sk_GENERAL_NAME_num( sanNames );
    
    // Check each name within the extension
    for( i = 0; i < sanNamesNb; i++ )
    {
      const GENERAL_NAME *currentName = sk_GENERAL_NAME_value( sanNames, i );

      if( currentName->type == GEN_DNS )
      {
        // Current name is a DNS name, let's check it
        char *dnsName = reinterpret_cast<char *>( ASN1_STRING_data( currentName->d.dNSName ) );

        // Make sure there isn't an embedded NUL character in the DNS name
        if( ASN1_STRING_length( currentName->d.dNSName ) == static_cast<int>( strlen( dnsName ) ) )
        {
          list.push_back( dnsName );
        }
      }
    }
    
    sk_GENERAL_NAME_pop_free( sanNames, GENERAL_NAME_free );
  }


  OpenSSLBase::HostnameValidationResult OpenSSLBase::matchesCommonName( const char *hostname,
                                                                        const X509 *serverCert )
  {
    int commonNameLoc = -1;
    X509_NAME_ENTRY* commonNameEntry = 0;
    ASN1_STRING* commonNameASN1 = 0;
    unsigned char* commonNameStr = 0;
    
    // Find the position of the CN field in the Subject field of the certificate
    commonNameLoc = X509_NAME_get_index_by_NID( X509_get_subject_name( static_cast<const X509*>( serverCert ) ),
                                                      NID_commonName, -1 );
    if( commonNameLoc < 0 )
    {
      return OpenSSLBase::Error;
    }
    
    // Extract the CN field
    commonNameEntry = X509_NAME_get_entry( X509_get_subject_name( static_cast<const X509*>( serverCert ) ),
                                              commonNameLoc );
    if( commonNameEntry == 0 )
    {
      return OpenSSLBase::Error;
    }
    
    // Convert the CN field to a C string
    commonNameASN1 = X509_NAME_ENTRY_get_data( commonNameEntry );
    if( commonNameASN1 == 0 )
    {
      return OpenSSLBase::Error;
    }
    commonNameStr = static_cast<unsigned char*>( ASN1_STRING_data( commonNameASN1 ) );
    
    // Make sure there isn't an embedded NUL character in the CN
    if( ASN1_STRING_length( commonNameASN1 ) != ( static_cast<int>( strlen( reinterpret_cast<char*>( commonNameStr ) ) ) ) )
    {
      return OpenSSLBase::MalformedCertificate;
    }
    
    // Compare expected hostname with the CN
    if( strcasecmp( hostname, reinterpret_cast<char*>( commonNameStr ) ) == 0 )
    {
      return OpenSSLBase::MatchFound;
    }
    else
    {
      return OpenSSLBase::MatchNotFound;
    }
  }

  OpenSSLBase::HostnameValidationResult OpenSSLBase::matchesSubjectAlternativeName( const char *hostname,
                                                                                    const X509 *serverCert )
  {
    OpenSSLBase::HostnameValidationResult result = MatchNotFound;
    int i;
    int sanNamesNb = -1;
    STACK_OF( GENERAL_NAME ) *sanNames = 0;
    
    // Try to extract the names within the SAN extension from the certificate
    sanNames = static_cast<stack_st_GENERAL_NAME *>( X509_get_ext_d2i( static_cast<const X509*>( serverCert ),
                                                                       NID_subject_alt_name,
                                                                       0, 0 ) );
    if( sanNames == 0 )
    {
      return OpenSSLBase::NoSANPresent;
    }
    sanNamesNb = sk_GENERAL_NAME_num( sanNames );
    
    // Check each name within the extension
    for( i = 0; i < sanNamesNb; i++ )
    {
      const GENERAL_NAME *currentName = sk_GENERAL_NAME_value( sanNames, i );
      
      if( currentName->type == GEN_DNS )
      {
        // Current name is a DNS name, let's check it
        char *dnsName = reinterpret_cast<char *>( ASN1_STRING_data( currentName->d.dNSName ) );

        // Make sure there isn't an embedded NUL character in the DNS name
        if( ASN1_STRING_length( currentName->d.dNSName ) != static_cast<int>( strlen( dnsName ) ) )
        {
          result = OpenSSLBase::MalformedCertificate;
          break;
        }
        else
        { // Compare expected hostname with the DNS name
          //std::cerr << "validate hostname found SAN" << dnsName  <<  std::endl;
          if( strcasecmp( hostname, dnsName ) == 0 )
          {
            result = OpenSSLBase::MatchFound;
            break;
          }
        }
      }
    }
    
    sk_GENERAL_NAME_pop_free( sanNames, GENERAL_NAME_free );
    
    return result;
  }

  OpenSSLBase::HostnameValidationResult OpenSSLBase::validateHostname( const char *hostname,
                                                                       const X509 *serverCert,
                                                                       StringList& listSAN )
  {
    OpenSSLBase::HostnameValidationResult result;
    
    if( ( hostname == 0 ) || ( serverCert == 0 ) )
      return OpenSSLBase::Error;
    
    // First try the Subject Alternative Names extension
    result = matchesSubjectAlternativeName( hostname, serverCert );
    if( result == OpenSSLBase::NoSANPresent )
    {
      // Extension was not found: try the Common Name
      //std::cerr << "validate hostname NoSANPresent" << std::endl;
      result = matchesCommonName( hostname, serverCert );
    }
    else
    {
      fillSubjectAlternativeName( listSAN, serverCert );
    }
    
    return result;
  }
}

#endif // HAVE_OPENSSL
