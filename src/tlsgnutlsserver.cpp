/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsgnutlsserver.h"

#ifdef HAVE_GNUTLS

#include <errno.h>

namespace gloox
{

  GnuTLSServer::GnuTLSServer( TLSHandler* th )
    : GnuTLSBase( th ), m_dhBits( 1024 )
  {
  }

  GnuTLSServer::~GnuTLSServer()
  {
    gnutls_certificate_free_credentials( m_x509cred );
    gnutls_dh_params_deinit( m_dhParams );
  }

  void GnuTLSServer::cleanup()
  {
    GnuTLSBase::cleanup();
    init();
  }

  bool GnuTLSServer::init( const std::string& clientKey,
                           const std::string& clientCerts,
                           const StringList& cacerts )
  {
    if( m_initLib && gnutls_global_init() != 0 )
      return false;

    if( gnutls_certificate_allocate_credentials( &m_x509cred ) < 0 )
      return false;

    setClientCert( clientKey, clientCerts );
    setCACerts( cacerts );

    generateDH();
    gnutls_certificate_set_dh_params( m_x509cred, m_dhParams );
    gnutls_certificate_set_rsa_export_params( m_x509cred, m_rsaParams);

    if( gnutls_init( m_session, GNUTLS_SERVER | GNUTLS_NONBLOCK ) != 0 )
    {
      gnutls_certificate_free_credentials( m_credentials );
      return false;
    }

    if( setPrios() != GNUTLS_E_SUCCESS )
      return false;

    gnutls_credentials_set( *m_session, GNUTLS_CRD_CERTIFICATE, m_x509cred );

    gnutls_certificate_server_set_request( *m_session, GNUTLS_CERT_REQUEST );

    gnutls_dh_set_prime_bits( *m_session, m_dhBits );

    gnutls_transport_set_ptr( *m_session, (gnutls_transport_ptr_t)this );
    gnutls_transport_set_push_function( *m_session, pushFunc );
    gnutls_transport_set_pull_function( *m_session, pullFunc );

    m_valid = true;
    return true;
  }

  void GnuTLSServer::setCACerts( const StringList& cacerts )
  {
    m_cacerts = cacerts;

    StringList::const_iterator it = m_cacerts.begin();
    for( ; it != m_cacerts.end(); ++it )
      gnutls_certificate_set_x509_trust_file( m_x509cred, (*it).c_str(), GNUTLS_X509_FMT_PEM );
  }

  void GnuTLSServer::setClientCert( const std::string& clientKey, const std::string& clientCerts )
  {
    m_clientKey = clientKey;
    m_clientCerts = clientCerts;

    if( !m_clientKey.empty() && !m_clientCerts.empty() )
    {
      gnutls_certificate_set_x509_key_file( m_x509cred,
                                            m_clientCerts.c_str(),
                                            m_clientKey.c_str(),
                                            GNUTLS_X509_FMT_PEM );
    }
  }


  void GnuTLSServer::generateDH()
  {
    gnutls_dh_params_init( &m_dhParams );
    gnutls_dh_params_generate2( m_dhParams, m_dhBits );
    gnutls_rsa_params_init( &m_rsaParams );
    gnutls_rsa_params_generate2( m_rsaParams, 512 );
  }

  void GnuTLSServer::getCertInfo()
  {
    m_certInfo.status = CertOk;

    getCommonCertInfo();

    m_valid = true;
  }

}

#endif // HAVE_GNUTLS
