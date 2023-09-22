/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsopensslclient.h"

#ifdef HAVE_OPENSSL

#if defined( _WIN32 ) 
void __cdecl ssl_message_trace( int write_dir, int version, int content_type,
                                const void *buf, size_t len, SSL *ssl, void *arg )
#else
void ssl_message_trace( int write_dir, int version, int content_type,
                        const void *buf, size_t len, SSL *ssl, void *arg )
#endif // _WIN32
{
  const char *prefix;
  const char *version_string;
  const char *content_type_string = "";
  const char *details1 = "";
  const char *details2 = "";

  prefix = write_dir ? ">" : "<";

  switch( version )
  {
    case SSL2_VERSION:
      version_string = "SSL 2.0";
      break;
    case SSL3_VERSION:
      version_string = "SSL 3.0";
      break;
    case TLS1_VERSION:
      version_string = "TLS 1.0";
      break;
    case TLS1_1_VERSION:
      version_string = "TLS 1.1";
      break;
    case TLS1_2_VERSION:
      version_string = "TLS 1.2";
      break;
    case TLS1_3_VERSION:
      version_string = "TLS 1.3";
      break;
    case DTLS1_VERSION:
      version_string = "DTLS 1.0";
      break;
    case DTLS1_2_VERSION:
      version_string = "DTLS 1.2";
      break;
    case DTLS1_BAD_VER:
      version_string = "DTLS 1.0 (bad)";
      break;
    default:
      version_string = "Unknown version";
  }

  if( version == SSL2_VERSION )
  {
    details1 = "???";

    if( len > 0 )
    {
      switch( ( static_cast<const unsigned char*>( buf ) )[0] )
      {
        case 0:
          details1 = ", ERROR:";
          details2 = " ???";
          if( len >= 3 )
          {
            unsigned err = ( ( static_cast<const unsigned char*>( buf ) )[1] << 8 ) + ( static_cast<const unsigned char*>( buf ) )[2];

            switch( err )
            {
              case 0x0001:
                details2 = " NO-CIPHER-ERROR";
                break;
              case 0x0002:
                details2 = " NO-CERTIFICATE-ERROR";
                break;
              case 0x0004:
                details2 = " BAD-CERTIFICATE-ERROR";
                break;
              case 0x0006:
                details2 = " UNSUPPORTED-CERTIFICATE-TYPE-ERROR";
                break;
            }
          }
          break;
        case 1:
          details1 = ", CLIENT-HELLO";
          break;
        case 2:
          details1 = ", CLIENT-MASTER-KEY";
          break;
        case 3:
          details1 = ", CLIENT-FINISHED";
          break;
        case 4:
          details1 = ", SERVER-HELLO";
          break;
        case 5:
          details1 = ", SERVER-VERIFY";
          break;
        case 6:
          details1 = ", SERVER-FINISHED";
          break;
        case 7:
          details1 = ", REQUEST-CERTIFICATE";
          break;
        case 8:
          details1 = ", CLIENT-CERTIFICATE";
          break;
      }
    }
  }

  if( version == SSL3_VERSION ||
      version == TLS1_VERSION ||
      version == TLS1_1_VERSION ||
      version == TLS1_3_VERSION ||
      version == TLS1_1_VERSION ||
      version == DTLS1_VERSION ||
      version == DTLS1_2_VERSION ||
      version == DTLS1_BAD_VER )
  {
    switch( content_type )
    {
      case 20:
        content_type_string = "ChangeCipherSpec";
        break;
      case 21:
        content_type_string = "Alert";
        break;
      case 22:
        content_type_string = "Handshake";
        break;
    }

    if( content_type == 21 ) /* Alert */
    {
      details1 = ", ???";

      if( len == 2 )
      {
        switch( ( static_cast<const unsigned char*>( buf ) )[0] )
        {
          case 1:
            details1 = ", warning";
            break;
          case 2:
            details1 = ", fatal";
            break;
        }

        details2 = " ???";
        switch( ( static_cast<const unsigned char*>( buf ) )[1] )
        {
          case 0:
            details2 = " close_notify";
            break;
          case 10:
            details2 = " unexpected_message";
            break;
          case 20:
            details2 = " bad_record_mac";
            break;
          case 21:
            details2 = " decryption_failed";
            break;
          case 22:
            details2 = " record_overflow";
            break;
          case 30:
            details2 = " decompression_failure";
            break;
          case 40:
            details2 = " handshake_failure";
            break;
          case 42:
            details2 = " bad_certificate";
            break;
          case 43:
            details2 = " unsupported_certificate";
            break;
          case 44:
            details2 = " certificate_revoked";
            break;
          case 45:
            details2 = " certificate_expired";
            break;
          case 46:
            details2 = " certificate_unknown";
            break;
          case 47:
            details2 = " illegal_parameter";
            break;
          case 48:
            details2 = " unknown_ca";
            break;
          case 49:
            details2 = " access_denied";
            break;
          case 50:
            details2 = " decode_error";
            break;
          case 51:
            details2 = " decrypt_error";
            break;
          case 60:
            details2 = " export_restriction";
            break;
          case 70:
            details2 = " protocol_version";
            break;
          case 71:
            details2 = " insufficient_security";
            break;
          case 80:
            details2 = " internal_error";
            break;
          case 90:
            details2 = " user_canceled";
            break;
          case 100:
            details2 = " no_renegotiation";
            break;
        }
      }
    }

    if( content_type == 22 ) /* Handshake */
    {
      details1 = "???";

      if( len > 0 )
      {
        switch( ( static_cast<const unsigned char*>( buf ) )[0] )
        {
          case 0:
            details1 = ", HelloRequest";
            break;
          case 1:
            details1 = ", ClientHello";
            break;
          case 2:
            details1 = ", ServerHello";
            break;
          case 11:
            details1 = ", Certificate";
            break;
          case 12:
            details1 = ", ServerKeyExchange";
            break;
          case 13:
            details1 = ", CertificateRequest";
            break;
          case 14:
            details1 = ", ServerHelloDone";
            break;
          case 15:
            details1 = ", CertificateVerify";
            break;
          case 3:
            details1 = ", HelloVerifyRequest";
            break;
          case 16:
            details1 = ", ClientKeyExchange";
            break;
          case 20:
            details1 = ", Finished";
            break;
        }
      }
    }
  }

  printf( "%s%s: %s%s%s\n", prefix,
          version_string, content_type_string, details1, details2 );

}

namespace gloox
{

  OpenSSLClient::OpenSSLClient( TLSHandler* th, const std::string& server )
    : OpenSSLBase( th, server )
  {
  }

  OpenSSLClient::~OpenSSLClient()
  {
  }

  bool OpenSSLClient::createCTX()
  {
    int ret = 0;
    switch( m_tlsVersion )
    {
// #ifndef OPENSSL_NO_SSL3_METHOD
//     case SSLv3:
//       m_ctx =  SSL_CTX_new( TLS_client_method() );
//       ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, SSL3_VERSION ) );
//       break;
// #endif // OPENSSL_NO_SSL3_METHOD
#ifndef OPENSSL_NO_TLS1_METHOD
    case TLSv1:
      m_ctx =  SSL_CTX_new( TLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, TLS1_VERSION ) );
      break;
#endif // OPENSSL_NO_TLS1_METHOD
#ifndef OPENSSL_NO_TLS1_1_METHOD
    case TLSv1_1:
      m_ctx =  SSL_CTX_new( TLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, TLS1_1_VERSION ) );
      break;
#endif // OPENSSL_NO_TLS1_1_METHOD
#ifndef OPENSSL_NO_TLS1_2_METHOD
    case TLSv1_2:
      m_ctx =  SSL_CTX_new( TLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, TLS1_2_VERSION ) );
      break;
#endif // OPENSSL_NO_TLS1_2_METHOD
#ifndef OPENSSL_NO_TLS1_3_METHOD
    case TLSv1_3:
      m_ctx =  SSL_CTX_new( TLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, TLS1_3_VERSION ) );
      break;
#endif // OPENSSL_NO_TLS1_3_METHOD
    case DTLSv1:
      m_ctx =  SSL_CTX_new( DTLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, DTLS1_VERSION ) );
      break;
    case DTLSv1_2:
      m_ctx =  SSL_CTX_new( DTLS_client_method() );
      ret = static_cast<int>( SSL_CTX_set_min_proto_version( m_ctx, DTLS1_2_VERSION ) );
      break;
    default:
      return false;
      break;
    }

    // Refuse connecting if SSL_CTX_set_min_proto_version() fails
    // FIXME: This fact should somehow be passed on the the caller
    if( !ret )
    {
      SSL_CTX_free( m_ctx );
      m_ctx = 0;
    }

    if( !m_ctx )
      return false;

    return true;
  }

  bool OpenSSLClient::hasChannelBinding() const
  {
    return true;
  }

  const std::string OpenSSLClient::channelBinding() const
  {
    unsigned char* buf[128];
    long res = SSL_get_finished( m_ssl, buf, 128 );
    return std::string( reinterpret_cast<char*>( buf ), res );
  }

  int OpenSSLClient::handshakeFunction()
  {
    return SSL_connect( m_ssl );
  }

}

#endif // HAVE_OPENSSL
