/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/



#include "tlsgnutlsserveranon.h"

#ifdef HAVE_GNUTLS

#include <errno.h>

namespace gloox
{

  GnuTLSServerAnon::GnuTLSServerAnon( TLSHandler* th )
    : GnuTLSBase( th ), m_dhBits( 1024 )
  {
  }

  GnuTLSServerAnon::~GnuTLSServerAnon()
  {
    gnutls_anon_free_server_credentials( m_anoncred );
    gnutls_dh_params_deinit( m_dhParams );
  }

  void GnuTLSServerAnon::cleanup()
  {
    GnuTLSBase::cleanup();
    init();
  }

  bool GnuTLSServerAnon::init( const std::string&,
                               const std::string&,
                               const StringList& )
  {
    if( m_initLib && gnutls_global_init() != 0 )
      return false;

    if( gnutls_anon_allocate_server_credentials( &m_anoncred ) < 0 )
      return false;

    generateDH();
    gnutls_anon_set_server_dh_params( m_anoncred, m_dhParams );

    if( gnutls_init( m_session, GNUTLS_SERVER ) != 0 )
      return false;

    if( setPrios( ":+ANON-ECDH:+ANON-DH" ) != GNUTLS_E_SUCCESS )
      return false;

    gnutls_credentials_set( *m_session, GNUTLS_CRD_ANON, m_anoncred );

    gnutls_dh_set_prime_bits( *m_session, m_dhBits );

    gnutls_transport_set_ptr( *m_session, static_cast<gnutls_transport_ptr_t>( this ) );
    gnutls_transport_set_push_function( *m_session, pushFunc );
    gnutls_transport_set_pull_function( *m_session, pullFunc );

    m_valid = true;
    return true;
  }

  void GnuTLSServerAnon::generateDH()
  {
    gnutls_dh_params_init( &m_dhParams );
    gnutls_dh_params_generate2( m_dhParams, m_dhBits );
  }

  void GnuTLSServerAnon::getCertInfo()
  {
    m_certInfo.status = CertOk;

    getCommonCertInfo();

    m_valid = true;
  }

}

#endif // HAVE_GNUTLS
