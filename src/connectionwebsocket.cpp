/*
 * Copyright (c) 2017-2023 by Jakob Schröter <js@camaya.net>
 * This file is part of the gloox library. http://camaya.net/gloox
 *
 * This software is distributed under a license. The full license
 * agreement can be found in the file LICENSE in this distribution.
 * This software may not be copied, modified, sold or distributed
 * other than expressed in the named license agreement.
 *
 * This software is distributed without any warranty.
 */

#if !defined( GLOOX_MINIMAL ) || defined( WANT_WEBSOCKET )

#include "config.h"

#include "gloox.h"

#include "connectionwebsocket.h"
#include "logsink.h"
#include "prep.h"
#include "tag.h"
#include "util.h"

#include <iomanip>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include <algorithm>

#ifdef DEBUG
#include <sstream>
#endif // DEBUG

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <stdio.h>
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
typedef unsigned long long uint64_t;
#else
#include <limits.h>
#include <string.h>
#include <unistd.h>
#endif

#define _DEBUG_DECODE_FLAG 1
#define _DEBUG_ENCODE_FLAG 2
#define _DEBUG_RAW_FLAG    4
#define _FIXED_MASK_FLAG   8

#define XMPP_MIN_HEADER_LENGTH 2

namespace gloox
{

  extern bool ci_equal( char ch1, char ch2 );
  extern std::string::size_type ci_find( const std::string& str1, const std::string& str2 );
  
  // ---- ConnectionWebSocket::FrameWebSocket ----
  ConnectionWebSocket::FrameWebSocket::FrameWebSocket( const LogSink& logInstance, int flags = _DEBUG_ENCODE_FLAG )
    : m_logInstance( logInstance ), m_flags( flags )
  {
    m_FIN = true;
    m_RSV1 = false;
    m_RSV2 = false;
    m_RSV3 = false;
    m_MASK = false;
    m_encodedFrame = 0;
    m_lenPayload = 0;
    m_len = 0;
    m_payloadOffset = 0;
  }
  
  ConnectionWebSocket::FrameWebSocket::FrameWebSocket( const LogSink& logInstance, const std::string& payload,
                                                       wsFrameType frameType, bool bMasked = true,
                                                       int flags = _DEBUG_ENCODE_FLAG )
    : m_logInstance( logInstance ), m_flags( flags )
  {
    m_FIN = true;
    m_RSV1 = false;
    m_RSV2 = false;
    m_RSV3 = false;
    m_MASK = bMasked;
    m_opcode = frameType;
    m_payload = payload;
    m_lenPayload =  static_cast<unsigned long>( payload.length() );
    m_len = 0;
    m_encodedFrame = 0;
    m_payloadOffset = 0;
  }
  
  unsigned long long ConnectionWebSocket::FrameWebSocket::getFrameLen()
  {
    return ( m_payloadOffset + m_lenPayload );
  }
  
  unsigned long long ConnectionWebSocket::FrameWebSocket::getEffectiveLen()
  {
    return ( m_len );
  }

  unsigned long long ConnectionWebSocket::FrameWebSocket::getOffsetLen()
  {
    return ( m_payloadOffset );
  }
  
  bool ConnectionWebSocket::FrameWebSocket::isFull()
  {
    return getFrameLen() <= getEffectiveLen();
  }

/**
 * @brief ConnectionWebSocket::FrameWebSocket::encode
 * @param addCRLF Flag to auto insert CRLF at end of payload data
 * @param status 16bits unsigned int describing the connection close reason
 */
void ConnectionWebSocket::FrameWebSocket::encode( bool addCRLF, int reason )
{
    int mskIdx = 0;
    long long len = 0;

    /*
      0                   1                   2                   3
      0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     +-+-+-+-+-------+-+-------------+-------------------------------+
     |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
     |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
     |N|V|V|V|       |S|             |   (if payload len==126/127)   |
     | |1|2|3|       |K|             |                               |
     +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
     |     Extended payload length continued, if payload len == 127  |
     + - - - - - - - - - - - - - - - +-------------------------------+
     |                               |Masking-key, if MASK set to 1  |
     +-------------------------------+-------------------------------+
     | Masking-key (continued)       |          Payload Data         |
     +-------------------------------- - - - - - - - - - - - - - - - +
     :                     Payload Data continued ...                :
     + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
     |                     Payload Data continued ...                |
     +---------------------------------------------------------------+


      Payload length:  7 bits, 7+16 bits, or 7+64 bits

         The length of the "Payload data", in bytes: if 0-125, that is the
         payload length.  If 126, the following 2 bytes interpreted as a
         16-bit unsigned integer are the payload length.  If 127, the
         following 8 bytes interpreted as a 64-bit unsigned integer (the
         most significant bit MUST be 0) are the payload length.  Multibyte
         length quantities are expressed in network byte order.  Note that
         in all cases, the minimal number of bytes MUST be used to encode
         the length, for example, the length of a 124-byte-long string
         can't be encoded as the sequence 126, 0, 124.  The payload length
         is the length of the "Extension data" + the length of the
         "Application data".  The length of the "Extension data" may be
         zero, in which case the payload length is the length of the
         "Application data".
      */

    if( addCRLF )
    {
      //include '\r\n'
      m_lenPayload += 2;
    }

    if( m_lenPayload < 126 )
    {
      len = m_lenPayload + 2;
    }
    else if( m_lenPayload < UINT_MAX )
    {
      len = m_lenPayload + 4;
    }
    else if( m_lenPayload < ULONG_MAX - 10 )
    {
      len = m_lenPayload + 10;
    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                         "ConnectionWebSocket::FrameWebSocket::encode Incorrect payload length" );
      return;
    }

    //if( m_MASK && m_lenPayload )
    if( m_MASK )
      len += 4;

    // reserve 2 bytes for status
    if( reason )
      len += 2;

    m_encodedFrame = new unsigned char[len];

    if( !m_encodedFrame )
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                         "ConnectionWebSocket::FrameWebSocket::encode Memory allocation failed" );
      return;
    }

    memset( m_encodedFrame, 0, len );

    m_encodedFrame[0] |= static_cast<unsigned char>( ( m_FIN ? 1 : 0 ) << 7 );
    m_encodedFrame[0] |= static_cast<unsigned char>( ( m_RSV1 ? 2 : 0 ) << 6 );
    m_encodedFrame[0] |= static_cast<unsigned char>( ( m_RSV2 ? 4 : 0 ) << 5 );
    m_encodedFrame[0] |= static_cast<unsigned char>( ( m_RSV3 ? 8 : 0 ) << 4 );
    m_encodedFrame[0] |= static_cast<unsigned char>( m_opcode );

    m_encodedFrame[1] |= static_cast<unsigned char>( ( m_MASK ? 1 : 0 ) << 7 );

    m_payloadOffset = 2;
    /*
    https://tools.ietf.org/html/rfc6455
    5.5.1.  Close

        The Close frame contains an opcode of 0x8.

          The Close frame MAY contain a body (the "Application data" portion of
          the frame) that indicates a reason for closing, such as an endpoint
          shutting down, an endpoint having received a frame too large, or an
          endpoint having received a frame that does not conform to the format
          expected by the endpoint.  If there is a body, the first two bytes of
          the body MUST be a 2-byte unsigned integer (in network byte order)
          representing a status code with value /code/ defined in Section 7.4.
          Following the 2-byte integer, the body MAY contain UTF-8-encoded data
          with value /reason/, the interpretation of which is not defined by
          this specification.  This data is not necessarily human readable but
          may be useful for debugging or passing information relevant to the
          script that opened the connection.  As the data is not guaranteed to
          be human readable, clients MUST NOT show it to end users.
     */
    if( reason )
    {
      m_lenPayload += 2;
      m_payload.insert( 0, 1, static_cast<char>( reason & 0x000000FF ) );
      m_payload.insert( 0, 1, static_cast<char>( (reason >> 8) & 0x000000FF) );
    }

    if( m_lenPayload )
    {
      if( m_lenPayload < 126 )
      {
        m_encodedFrame[1] |= static_cast<unsigned char>( m_lenPayload & 0x7F );

        if( m_MASK )
        {
          mskIdx = 2;
          m_payloadOffset += 4;

          std::string mask;

          getMask( mask );
          for( unsigned long long k = 0; k < m_payload.size(); k++ )
          {
            if( ( m_flags & _FIXED_MASK_FLAG ) != _FIXED_MASK_FLAG )
              m_encodedFrame[m_payloadOffset+k] = m_payload[k] ^ ( static_cast<char>( mask[k % 4] ) );
            else
              m_encodedFrame[m_payloadOffset+k] = static_cast<unsigned char>( m_payload[k] ^ 0xFF );
          }

          if( ( m_flags & _FIXED_MASK_FLAG ) != _FIXED_MASK_FLAG )
          {
            m_encodedFrame[mskIdx] = static_cast<char>( mask[0] & 0xFF );
            m_encodedFrame[mskIdx+1] = static_cast<char>( mask[1] & 0xFF );
            m_encodedFrame[mskIdx+2] = static_cast<char>( mask[2] & 0xFF );
            m_encodedFrame[mskIdx+3] = static_cast<char>( mask[3] & 0xFF );

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ mask[( m_lenPayload - 2 ) % 4];
              m_encodedFrame[len-1] = 0x0a ^ mask[( m_lenPayload - 1 ) % 4];
            }
          }
          else
          {
            m_encodedFrame[mskIdx] = 0xFF;
            m_encodedFrame[mskIdx+1] = 0xFF;
            m_encodedFrame[mskIdx+2] = 0xFF;
            m_encodedFrame[mskIdx+3] = 0xFF;

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ 0xFF;
              m_encodedFrame[len-1] = 0x0a ^ 0xFF;
            }
          }
        }
        else
          memcpy( &m_encodedFrame[m_payloadOffset], m_payload.c_str(), m_lenPayload );

      }
      else if( m_lenPayload < UINT_MAX )
      {
        m_encodedFrame[1] |= 0x7E;
        m_encodedFrame[2] = ( ( m_lenPayload >> 8 ) & 0xFF );
        m_encodedFrame[3] = ( m_lenPayload & 0xFF );
        m_payloadOffset = 4;

        if( m_MASK )
        {
          mskIdx = 4;
          m_payloadOffset += 4;

          std::string mask;

          getMask( mask );

          for( unsigned long long k = 0; k < m_payload.size(); k++ )
          {
            if( ( m_flags & _FIXED_MASK_FLAG ) != _FIXED_MASK_FLAG )
              m_encodedFrame[m_payloadOffset+k] = m_payload[k] ^ ( static_cast<char>( mask[k%4] ) );
            else
              m_encodedFrame[m_payloadOffset+k] = static_cast<unsigned char>( m_payload[k] ^ 0xFF );
          }

          if( (m_flags & _FIXED_MASK_FLAG) != _FIXED_MASK_FLAG )
          {
            m_encodedFrame[mskIdx] = static_cast<char>( mask[0] & 0xFF );
            m_encodedFrame[mskIdx+1] = static_cast<char>( mask[1] & 0xFF );
            m_encodedFrame[mskIdx+2] = static_cast<char>( mask[2] & 0xFF );
            m_encodedFrame[mskIdx+3] = static_cast<char>( mask[3] & 0xFF );

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ mask[( m_lenPayload - 2 ) % 4];
              m_encodedFrame[len-1] = 0x0a ^ mask[( m_lenPayload - 1 ) % 4];
            }
          }
          else
          {
            m_encodedFrame[mskIdx] = 0xFF;
            m_encodedFrame[mskIdx+1] = 0xFF;
            m_encodedFrame[mskIdx+2] = 0xFF;
            m_encodedFrame[mskIdx+3] = 0xFF;

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ 0xFF;
              m_encodedFrame[len-1] = 0x0a ^ 0xFF;
            }
          }
        }
        else
          memcpy( &m_encodedFrame[m_payloadOffset], m_payload.c_str(), m_lenPayload );

      }
      else if( m_lenPayload < ULONG_MAX )
      {
        m_encodedFrame[1] |= 0x7F;
        m_encodedFrame[9] = ( m_lenPayload & 0x000000FF );
        m_encodedFrame[8] = ( ( m_lenPayload >> 8 ) & 0x00000000000000FF );
        m_encodedFrame[7] = ( ( m_lenPayload >> 16 ) & 0x00000000000000FF );
        m_encodedFrame[6] = ( ( m_lenPayload >> 24 ) & 0x00000000000000FF );
        m_encodedFrame[5] = ( ( m_lenPayload >> 32 ) & 0x00000000000000FF );
        m_encodedFrame[4] = ( ( m_lenPayload >> 40 ) & 0x00000000000000FF );
        m_encodedFrame[3] = ( ( m_lenPayload >> 48 ) & 0x00000000000000FF );
        m_encodedFrame[2] = static_cast<unsigned char>( ( ( m_lenPayload >> 56 ) & 0x00000000000000FF ) );
        m_payloadOffset = 10;

        if( m_MASK )
        {
          mskIdx = 10;
          m_payloadOffset += 4;

          std::string mask;

          getMask( mask );

          for( unsigned long long k = 0; k < m_payload.size(); k++ )
          {
            if( ( m_flags & _FIXED_MASK_FLAG ) != _FIXED_MASK_FLAG )
              m_payload[k] ^= mask[k%4];
            else
              m_encodedFrame[m_payloadOffset+k] = static_cast<unsigned char>( m_payload[k] ^ 0xFF );
          }

          if( ( m_flags & _FIXED_MASK_FLAG ) != _FIXED_MASK_FLAG )
          {
            memcpy( &m_encodedFrame[mskIdx], mask.c_str(), 4 );
            memcpy( &m_encodedFrame[m_payloadOffset], m_payload.c_str(), m_lenPayload );

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ mask[( m_lenPayload - 2 ) % 4];
              m_encodedFrame[len-1] = 0x0a ^ mask[( m_lenPayload - 1 ) % 4];
            }
          }
          else
          {
            m_encodedFrame[mskIdx] = 0xFF;
            m_encodedFrame[mskIdx+1] = 0xFF;
            m_encodedFrame[mskIdx+2] = 0xFF;
            m_encodedFrame[mskIdx+3] = 0xFF;

            if( addCRLF )
            {
              m_encodedFrame[len-2] = 0x0d ^ 0xFF;
              m_encodedFrame[len-1] = 0x0a ^ 0xFF;
            }
          }
        }
        else
          memcpy( &m_encodedFrame[m_payloadOffset], m_payload.c_str(), m_lenPayload );

      }
    }

    if( !m_lenPayload && m_MASK )
    {
      mskIdx = 2;
      if( ( m_flags & _FIXED_MASK_FLAG) != _FIXED_MASK_FLAG )
      {
        std::string mask;

        getMask(mask);

        m_encodedFrame[mskIdx] = static_cast<char>( mask[0] & 0xFF );
        m_encodedFrame[mskIdx+1] = static_cast<char>( mask[1] & 0xFF );
        m_encodedFrame[mskIdx+2] = static_cast<char>( mask[2] & 0xFF );
        m_encodedFrame[mskIdx+3] = static_cast<char>( mask[3] & 0xFF );
      }
      else
      {
        m_encodedFrame[mskIdx] = 0xFF;
        m_encodedFrame[mskIdx+1] = 0xFF;
        m_encodedFrame[mskIdx+2] = 0xFF;
        m_encodedFrame[mskIdx+3] = 0xFF;
      }
    }

    if( addCRLF && !m_MASK )
    {
      m_encodedFrame[len-2] = 0x0d;
      m_encodedFrame[len-1] = 0x0a;
    }

#ifdef DEBUG
    if( ( m_flags & _DEBUG_ENCODE_FLAG ) == _DEBUG_ENCODE_FLAG )
    {
      if( m_encodedFrame )
      {
        std::stringstream logStream;
        logStream << "Display encoded frame to send [length =" << len << "]";
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, logStream.str() );
        logStream.str( "" );

        for( int i = 0; i < len; i++ )
        {
          logStream << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)m_encodedFrame[i] << ' ';
        }
        m_logInstance.dbg( LogAreaClassConnectionWebSocket,logStream.str() );
      }
    }
#endif // DEBUG

  }

  void ConnectionWebSocket::FrameWebSocket::decode( std::string& frameContent )
  {
    const char* frameBuffer = frameContent.c_str();
    unsigned char mask[4];
    m_len = frameContent.length();
    
    if( frameBuffer )
    {
      m_FIN = ( frameBuffer[0] & 0x80 ) > 0 ? true : false;
      m_RSV1 = ( frameBuffer[0] & 0x40 ) > 0 ? true : false;
      m_RSV2 = ( frameBuffer[0] & 0x20 ) > 0 ? true : false;
      m_RSV3 = ( frameBuffer[0] & 0x10 ) > 0 ? true : false;
      m_opcode = static_cast<FrameWebSocket::wsFrameType>( frameBuffer[0] & 0x0F );
      m_MASK = ( frameBuffer[1] & 0x80 ) > 0 ? true : false;
      m_lenPayload = ( frameBuffer[1] & 0x7F );

      m_payloadOffset = ( m_MASK ? 6 : 2 );
      
      if( m_lenPayload == 126 )
      {
        m_lenPayload = static_cast<int>( frameBuffer[3] & 0xFF ) + ( static_cast<int>( frameBuffer[2] & 0xFF ) ) * 256;
        m_payloadOffset = ( m_MASK ? 8 : 4 );
      }
      else if( m_lenPayload == 127 )
      {
        m_lenPayload = frameBuffer[9];
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[8] ) ) << 8;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[7] ) ) << 16;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[6] ) ) << 24;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[5] ) ) << 32;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[4] ) ) << 40;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[3] ) ) << 48;
        m_lenPayload |= ( static_cast<uint64_t>( frameBuffer[2] ) ) << 56;
        m_payloadOffset = (m_MASK ? 14 : 10);
      }
      
      if( m_MASK )
      {
        int maskOffset = m_payloadOffset - 4;
        memcpy( &mask[0], &frameBuffer[maskOffset], 4 );
      }

      long long unsigned int payloadRealSize = m_len - m_payloadOffset;
      if( payloadRealSize > 0 )
      {
        unsigned char *lFrame = new unsigned char[payloadRealSize + 1];
        memcpy( &( lFrame[0] ), frameBuffer + m_payloadOffset, payloadRealSize );

        lFrame[payloadRealSize] = 0;

        if( m_MASK )
        {
          for( unsigned long long k = 0; k < payloadRealSize; k++ )
          {
            lFrame[k] ^= mask[k%4];
          }
        }

        m_payload.erase();
        m_payload.append( reinterpret_cast<const char*>( lFrame ), payloadRealSize );

        delete[] lFrame;
      }
    }
  }

  void ConnectionWebSocket::FrameWebSocket::getMask( std::string& mask )
  {
    const static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
    
    int l = static_cast<int>( sizeof( charset ) - 1 );
    char key;

    if( mask.size() < 4 )
      mask.resize( 4 );

    for( int n = 0; n < 4; n++ )
    {
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
      key = rand() % 70;
#else
      key = static_cast<char>( rand() % l );
#endif
      mask[n] = charset[static_cast<int>( key )];
    }

#ifdef DEBUG
    if( ( m_flags & _DEBUG_ENCODE_FLAG ) == _DEBUG_ENCODE_FLAG )
    {
      std::stringstream logStream;
      logStream << "Display Mask value :";
      for( int i = 0; i < 4; i++ )
      {
        logStream << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)mask[i] << ' ';
      }

      m_logInstance.dbg( LogAreaClassConnectionWebSocket,logStream.str() );
    }
#endif // DEBUG

  }
  
#ifdef DEBUG
  void ConnectionWebSocket::FrameWebSocket::dump()
  {
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, std::string( "NEW FRAME DUMP" ) );
    m_logInstance.dbg(LogAreaClassConnectionWebSocket, std::string( "Last Fragment: " ) + ( m_FIN ? "yes" : "no" ) );
    m_logInstance.dbg(LogAreaClassConnectionWebSocket, std::string( "Masked       : " ) + ( m_MASK ? "yes" : "no" ) );
    char length[21];
    sprintf( length, "%lld", getFrameLen() );
    length[20] = 0;
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, std::string( "Frame Length : " ) + (char*)length);
    switch( m_opcode )
    {
      case wsFrameContinuation:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : Continuation" );
        break;
      case wsFrameText:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : Text" );
        break;
      case wsFrameBinary:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : Binary" );
        break;
      case wsFrameClose:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : CloseConnection" );
        break;
      case wsFramePing:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : Ping" );
        break;
      case wsFramePong:
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Type         : Pong" );
        break;
    }
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Frame Payload: " + m_payload );
  }
#endif // DEBUG
  // ---- ~ConnectionWebSocket::FrameWebSocket ----

  // ---- ConnectionWebSocket ----

  ConnectionWebSocket::ConnectionWebSocket( ConnectionBase* connection, const LogSink& logInstance,
                                            const std::string& wsHost,
                                            PingHandler *ph )
    : ConnectionBase( 0 ), m_logInstance( logInstance ), m_wsstate( wsStateDisconnected ),
      m_pinghandler( ph ), m_path( "/xmpp" ), m_bufferContentLength( 0 ),
      m_parser( this ), m_streamInitiationValidated( false ), m_streamClosureValidated( false ),
      m_bMasked( true ), m_flags( _DEBUG_DECODE_FLAG ),
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
      m_hCryptProv( 0 )
#else
      m_fd_random( -1 )
#endif // _WIN32
  {
    m_server = wsHost;
    initInstance( connection );
  }
  
  ConnectionWebSocket::ConnectionWebSocket( ConnectionDataHandler* cdh, ConnectionBase* connection,
                                            const LogSink& logInstance, const std::string& wsHost,
                                            PingHandler *ph )
    : ConnectionBase( cdh ), m_logInstance( logInstance ), m_wsstate( wsStateDisconnected ),
      m_pinghandler( ph ), m_path( "/xmpp" ), m_bufferContentLength( 0 ),
      m_parser( this ), m_streamInitiationValidated( false ), m_streamClosureValidated( false ),
      m_bMasked( true ), m_flags( _DEBUG_DECODE_FLAG ),
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
      m_hCryptProv ( 0 )
#else
      m_fd_random ( -1 )
#endif // _WIN32
  {
    m_server = wsHost;
    initInstance( connection );
  }
  
  void ConnectionWebSocket::initInstance( ConnectionBase* connection )
  {
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
    if( m_hCryptProv == 0 )
    {
      if( !CryptAcquireContext( &m_hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )
      {
        m_logInstance.err( LogAreaClassConnectionWebSocket, "Unable to acquire crypto context" );
      }
    }
#else
    if( m_fd_random == -1 )
    {
      m_fd_random = open( SYSTEM_RANDOM_FILEPATH, O_RDONLY );
      if( m_fd_random < 0 )
      {
        std::string errMsg;

        errMsg = "Unable to open random device ";
        errMsg += SYSTEM_RANDOM_FILEPATH;
        errMsg += " : error = ";
        errMsg += static_cast<char>( m_fd_random );

        m_logInstance.err( LogAreaClassConnectionWebSocket, errMsg );
      }
      else
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Random device opened successfully" );

    }
#endif // _WIN32

    if( connection )
    {
      connection->registerConnectionDataHandler( this );
      m_connection = connection;
    }
    
    m_buffer.erase();
    m_fragmentBuffer.erase();
    
    m_streamInitiationValidated = false;
    m_streamClosureValidated = false;
  }
  
  ConnectionWebSocket::~ConnectionWebSocket()
  {
    m_streamInitiationValidated = false;
#if !( defined( _WIN32 ) || defined ( _WIN32_WCE ) )
    close( m_fd_random );
#endif

    if( m_connection )
      delete m_connection;

  }
  
  ConnectionBase* ConnectionWebSocket::newInstance() const
  {
    ConnectionBase* conn = m_connection ? m_connection->newInstance() : 0;
    return new ConnectionWebSocket( m_handler, conn, m_logInstance, m_server,
                                    m_pinghandler );
  }
  
  void ConnectionWebSocket::disconnect()
  {
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Disconnecting." );
    sendCloseWebSocketFrame( wsStatusAway );
  }
  
  ConnectionError ConnectionWebSocket::connect( int timeout )
  {
    m_timeout = timeout;
    if( m_state >= StateConnecting )
      return ConnNoError;
    
    if( !m_handler )
      return ConnNotConnected;
    
    m_state = StateConnecting;
    m_wsstate = wsStateConnecting;
    
    if( m_connection )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Initiating Websocket connection to server: " );
      return m_connection->connect( timeout );
    }
    else
      return ConnNotConnected; // FIXME?
  }

  void ConnectionWebSocket::cleanup()
  {
    m_state = StateDisconnected;
    m_wsstate = wsStateDisconnected;
    m_streamInitiationValidated = false;
    
    m_clientOpeningHandshake.empty();
    
    if( m_connection )
    {
      m_connection->cleanup();
    }
  }
  
  ConnectionError ConnectionWebSocket::recv( int timeout )
  {
    ConnectionError ret = ConnNoError;
    
    if( m_state == StateDisconnected )
      return ConnNotConnected;
    
    if( m_connection )
    {
      ret = m_connection->recv( timeout );
    }
    
    return ret;
  }

  ConnectionError ConnectionWebSocket::receive()
  {
    if( m_connection )
      return m_connection->receive();
    else
      return ConnNotConnected;
  }
  
  bool ConnectionWebSocket::send( const char* data, const size_t len )
  {
    
    if( m_state == StateDisconnected )
      return false;
    
//     m_logInstance.dbg( LogAreaClassConnectionWebSocket, std::string( "ConnectionWebSocket::send : \n" )
//                                                           + std::string( data ) );

    std::string frameContent( data, len );
    FrameWebSocket frame( m_logInstance, frameContent, FrameWebSocket::wsFrameText, m_bMasked, m_flags );
    
    // TODO: Check if the flag CRLF must or not be enabled here.
    //       Maybe something dealing with ejabber compativility...
    frame.encode( false );
    
    return sendFrame( frame );
  }
  
  bool ConnectionWebSocket::send( const std::string& data )
  {
    
    if( m_state == StateDisconnected )
      return false;
    
//     m_logInstance.dbg( LogAreaClassConnectionWebSocket, "ConnectionWebSocket::send : \n" + data );

    std::string frameContent = data;
    FrameWebSocket frame( m_logInstance, frameContent, FrameWebSocket::wsFrameText, m_bMasked, m_flags );
    
    // TODO: Check if the flag CRLF must or not be enabled here.
    //       Maybe something dealing with ejabber compativility...
    frame.encode( false );

    return sendFrame( frame );
  }
  
  void ConnectionWebSocket::getStatistics( long int& totalIn, long int& totalOut )
  {
    if( m_connection )
    {
      m_connection->getStatistics( totalIn, totalOut );
    }
    else
    {
      totalIn = 0;
      totalOut = 0;
    }
  }
  
  std::string ConnectionWebSocket::header( std::string to, std::string xmlns, std::string xmllang )
  {
//     m_logInstance.dbg( LogAreaClassConnectionWebSocket, "ConnectionWebSocket::header");
    // we can consisder that the stream restarts
    // m_state = StateConnected;
    if( m_wsstate >= wsStateClientOpeningHandshake )
    {
      m_wsstate = wsStateClientOpeningXmpp;
      
      std::string head = "<open xmlns='urn:ietf:params:xml:ns:xmpp-framing' to='";
      head += m_server;
      head += "' version='1.0' />";
      return head;
    }
    
    return "";
  }
  
  const std::string ConnectionWebSocket::getHTTPField( const std::string& field )
  {
    std::string::size_type fp = ci_find( m_bufferHeader, "\r\n" + field + ": " );
    
    if( fp == std::string::npos )
    {
      fp = ci_find( m_bufferHeader, "\r\n" + field + ":" );
      if( fp == std::string::npos )
      {
        return EmptyString;
      }
      else
        fp += field.length() + 3;
      
    }
    else
      fp += field.length() + 4;
    
    const std::string::size_type fp2 = m_bufferHeader.find( "\r\n", fp );
    if( fp2 == std::string::npos )
      return EmptyString;
    
    return m_bufferHeader.substr( fp, fp2 - fp );
  }
  
  void ConnectionWebSocket::handleReceivedData( const ConnectionBase* /*connection*/,
                                                const std::string& data )
  {
    if( !m_connection || !m_handler )
      return;
    
#ifdef DEBUG
    if( ( m_flags & _DEBUG_RAW_FLAG ) ==  _DEBUG_RAW_FLAG )
    {
      std::stringstream ost1, ost2;
      unsigned int i = 0;

      for( i = 0; i < data.size(); )
      {
        ost1 << data.at( i );
        ost2 << std::setfill( '0' ) << std::setw( 2 ) << std::hex << (int)data.at( i ) << ' ';
        i++;
      }

      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "ConnectionWebSocket::handleReceivedData data: \n" + ost1.str() );
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "ConnectionWebSocket::handleReceivedData data: \n" + ost2.str() );

      ost1.clear();
      ost2.clear();
      ost1.str( std::string() );
      ost2.str( std::string() );
    }
#endif // DEBUG

    m_buffer += data;

    switch( m_wsstate )
    {
      case wsStateClientOpeningHandshake:
      {
        m_streamInitiationValidated = false;
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Checking HTTP protocol..." );
        if( m_buffer.find("HTTP") == 0 )
        {
          m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Found HTTP protocol..." );
          std::string::size_type headerLength = 0;
          while( ( headerLength = m_buffer.find( "\r\n\r\n" ) ) != std::string::npos )
          {
            m_bufferHeader = m_buffer.substr( 0, headerLength + 2 );
//             m_logInstance.dbg( LogAreaClassConnectionWebSocket, "bufferHeader: " + m_bufferHeader );

            const std::string& statusCode = m_bufferHeader.substr( 9, 3 );
            if( statusCode != "101" )
            {
              m_logInstance.warn( LogAreaClassConnectionWebSocket, "Received error via legacy HTTP status code: "
                                                                    + statusCode + ". Disconnecting." );
              m_wsstate = wsStateDisconnecting;
              sendCloseWebSocketFrame( wsStatusProtocolError );
              break;
            }

            if( !( getHTTPField( "Upgrade" ) == "websocket" ) || !( getHTTPField( "Connection" ) == "Upgrade" ) )
            {
              m_logInstance.warn( LogAreaClassConnectionWebSocket,
                                    "Received error in header Upgrade or Connection. Disconnecting." );
              m_wsstate = wsStateDisconnecting;
              sendCloseWebSocketFrame( wsStatusProtocolError );
              break;
            }

            if( getHTTPField( "Sec-WebSocket-Protocol" ).find( "xmpp" ) == std::string::npos )
            {
              m_logInstance.warn( LogAreaClassConnectionWebSocket,
                                    "Received error in header Sec-WebSocket-Protocol. Disconnecting." );
              m_wsstate = wsStateDisconnecting;
              sendCloseWebSocketFrame( wsStatusMandatoryExtension );
              break;
            }

            std::string sWebsocketAccept = m_clientOpeningHandshake + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
            SHA sha;
            std::string key_ = sWebsocketAccept;
            sha.feed( key_ );
            key_ = sha.binary();

            sha.reset();

            m_logInstance.err( LogAreaClassConnectionWebSocket, "Checking Sec-WebSocket-Accept "
                                                                  + getHTTPField( "Sec-WebSocket-Accept" )
                                                                  + " against " + Base64::encode64( key_ ) );
            if( getHTTPField( "Sec-WebSocket-Accept" ) != Base64::encode64( key_ ) )
            {
              m_logInstance.warn( LogAreaClassConnectionWebSocket,
                                    "Received error in header Sec-WebSocket-Accept. Received: "
                                      + getHTTPField( "Sec-WebSocket-Accept" )
                                      + ", Expected: " + Base64::encode64( key_ ) + ". Disconnecting." );
              m_wsstate = wsStateDisconnecting;
              sendCloseWebSocketFrame(wsStatusProtocolError);
              break;
            }

            m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                                "Sec-WebSocket-Accept header validated. Switching." );
            m_buffer.erase( 0, headerLength + 4 + m_bufferContentLength );
            m_bufferContentLength = 0;
            m_bufferHeader = EmptyString;
            srand( static_cast<unsigned int>( time( 0 ) ) );
            if( sendOpenXmppStream() )
            {
              m_wsstate = wsStateClientOpeningXmpp;
            }
            else
            {
              m_wsstate = wsStateDisconnecting;
              sendCloseWebSocketFrame( wsStatusInternalServerError );
            }
          }
        }
        break;
      }
      case wsStateClientOpeningXmpp:
      {
        if( m_buffer.length() < XMPP_MIN_HEADER_LENGTH )
        {
          m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                              "wsState = wsStateClientOpeningXmpp; Data length too small, skip it!" );
          return;
        }

        FrameWebSocket frame( m_logInstance, m_flags );
        m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                            "wsState = wsStateClientOpeningXmpp; Checking response opening xmpp stream ");

        frame.decode( m_buffer );
//         frame.dump();

        if( !frame.isFull() )
          return;

        if( frame.getType() == FrameWebSocket::wsFrameText )
        {
          if( frame.isUnfragmented() )
          {
#if 0
          std::stringstream logStream;
          logStream << "EffectiveLen = "<< frame.getEffectiveLen() <<", OffsetLen = " << frame.getOffsetLen();
          m_logInstance.dbg( LogAreaClassConnectionWebSocket, logStream.str());
#endif // 0
            std::string payload = frame.getPayload();

            // Do special stuff for websocket and call standard data handler if parsed OK
            if( parse( payload ) && m_handler )
            {
              m_handler->handleReceivedData( this, payload );
            }
          }
          else
          {
            m_fragmentBuffer.append( frame.getPayload() );
            m_wsstate = wsStateClientOpeningXmppFragmented;
            m_buffer.erase();
            m_bufferHeader = EmptyString;
            break;
          }
        }

        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "XMPP session started" );

        m_state = StateConnected;

        m_buffer.erase( 0, frame.getFrameLen() );
        m_bufferHeader = EmptyString;
        m_wsstate= wsStateConnected;

        if( m_buffer.length() > 0 )
        {
          std::string data( m_buffer );
          m_buffer.erase();
          handleReceivedData( this, data );
        }
        break;
      }
      case wsStateClientOpeningXmppFragmented:
      {
        while( m_buffer.length() > 0 )
        {

          if( m_buffer.length() < XMPP_MIN_HEADER_LENGTH )
          {
            m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                              "wsState = wsStateClientOpeningXmppFragmented; Data length too small, skip it" );
            return;
          }

          FrameWebSocket frame( m_logInstance, m_flags );
//           m_logInstance.dbg( LogAreaClassConnectionWebSocket, "wsState = wsStateConnected " );

          frame.decode( m_buffer );
//           frame.dump();

          // Check if decoded buffer contains a full frame, else return and wait for a complete buffer
          if( !frame.isFull() )
            return;

          if( frame.getType() == FrameWebSocket::wsFrameContinuation )
          {

            m_buffer.erase( 0, frame.getFrameLen() );

            // This is the last frame of a fragmented buffer, FIN bit is set
            if( frame.isUnfragmented() )
            {
              m_fragmentBuffer.append( frame.getPayload() );

              // Do special stuff for websocket and call standard data handler if parsed OK
              if( parse( m_fragmentBuffer ) && m_handler )
              {
                m_handler->handleReceivedData( this, m_fragmentBuffer );
              }

              m_fragmentBuffer.erase();
              m_logInstance.dbg( LogAreaClassConnectionWebSocket, "XMPP session started" );

              m_state = StateConnected;

              m_bufferHeader = EmptyString;
              m_wsstate= wsStateConnected;

              if( m_buffer.length() > 0 )
              {
                std::string data( m_buffer );
                m_buffer.erase();
                handleReceivedData( this, data );
              }
            }
            else
            {
              m_fragmentBuffer.append( frame.getPayload() );
            }
          }
          else
          {
            m_buffer.erase();
            m_bufferHeader = EmptyString;
            m_fragmentBuffer.erase();
            sendCloseXmppStream( ConnStreamError );
            m_wsstate = wsStateClientClosingXmpp;
          }
        }
        break;
      }
      case wsStateClientClosingXmpp:
      {
        FrameWebSocket frame( m_logInstance, m_flags );
        m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                          "wsState = wsStateClientClosingXmpp; Checking response closing xmpp stream " );

        frame.decode( m_buffer );
//         frame.dump();

        if( frame.getType() == FrameWebSocket::wsFrameText )
        {
          std::string payload = frame.getPayload();

          // Do special stuff for websocket and call standard data handler if parsed OK
          if( parse( payload ) && m_handler )
          {
            m_handler->handleReceivedData( this, payload );
          }
        }

        sendCloseWebSocketFrame( wsStatusNormal );
        m_wsstate = wsStateDisconnecting;
        break;
      }
      case wsStateConnected:
      {
        while( m_buffer.length() > 0 )
        {

          if( m_buffer.length() < XMPP_MIN_HEADER_LENGTH )
          {
            m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                              "wsState = wsStateConnected; Data length too small, skip it" );
            return;
          }

          FrameWebSocket frame( m_logInstance, m_flags );
          frame.decode( m_buffer );
//           frame.dump();

          //Check if decoded buffer contains a full frame, else return and wait for a complet buffer
          if( !frame.isFull() )
            return;

          if( frame.getType() == FrameWebSocket::wsFrameText )
          {
            if( frame.isUnfragmented() )
            {
              m_buffer.erase( 0, frame.getFrameLen() );
              m_bufferHeader = EmptyString;

              if( !m_handler )
                return;

              m_handler->handleReceivedData( this, frame.getPayload() );
            }
            else
            {
              m_buffer.erase( 0, frame.getFrameLen() );
              m_fragmentBuffer.append( frame.getPayload() );
              m_wsstate = wsStateConnectedFragmented;
              //m_buffer.erase();
              m_bufferHeader = EmptyString;
              break;
            }
          }
          else if( frame.getType() == FrameWebSocket::wsFramePing )
          {
            m_buffer.erase( 0, frame.getFrameLen() );
            std::string pingBody = frame.getPayload();
            if( m_pinghandler )
              m_pinghandler->handlePing( PingHandler::websocketPing, pingBody );

            sendPongWebSocketFrame( pingBody );
          }
          else if( frame.getType() == FrameWebSocket::wsFramePong )
          {
            m_buffer.erase();
            std::string pongBody = frame.getPayload();
            if( m_pinghandler )
              m_pinghandler->handlePing( PingHandler::websocketPong, pongBody );
          }
          else if( frame.getType() == FrameWebSocket::wsFrameClose )
          {
            m_buffer.erase( 0, frame.getFrameLen() );
            sendCloseWebSocketFrame( wsStatusNormal );
            m_wsstate = wsStateDisconnected;
            m_state = StateDisconnected;
          }
          else
          {
            m_buffer.erase();
            m_bufferHeader = EmptyString;
            m_wsstate = wsStateDisconnecting;
            sendCloseWebSocketFrame( wsStatusProtocolError );
          }
        }
        break;
      }
      case wsStateConnectedFragmented:
      {
        while( m_buffer.length() > 0 )
        {

          if( m_buffer.length() < XMPP_MIN_HEADER_LENGTH )
          {
            m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                              "wsState = wsStateConnectedFragmented; Data length too small, skip it" );
            return;
          }

          FrameWebSocket frame( m_logInstance, m_flags );
          frame.decode( m_buffer );
//           frame.dump();

          //Check if decoded buffer contains a full frame, else return and wait for a complet buffer
          if( !frame.isFull() )
            return;

          m_buffer.erase( 0, frame.getFrameLen() );

          if( frame.getType() == FrameWebSocket::wsFrameContinuation )
          {
            if( frame.isUnfragmented() )
            {
              m_fragmentBuffer.append( frame.getPayload() );
              m_wsstate = wsStateConnected;
              if( m_handler )
              {
                m_handler->handleReceivedData( this, m_fragmentBuffer );
              }
              m_fragmentBuffer.erase();
            }
            else
            {
              m_fragmentBuffer.append( frame.getPayload() );
            }
          }
          else if( frame.getType() == FrameWebSocket::wsFramePing )
          {
            std::string pingBody = frame.getPayload();
            if( m_pinghandler )
              m_pinghandler->handlePing( PingHandler::websocketPing, pingBody );

            sendPongWebSocketFrame( pingBody );
          }
          else if( frame.getType() == FrameWebSocket::wsFramePong )
          {
            std::string pongBody = frame.getPayload();
            if( m_pinghandler )
              m_pinghandler->handlePing( PingHandler::websocketPong, pongBody );
          }
          else if( frame.getType() == FrameWebSocket::wsFrameClose )
          {
            m_fragmentBuffer.erase();
            sendCloseWebSocketFrame( wsStatusNormal );
            m_wsstate = wsStateDisconnected;
            m_state = StateDisconnected;
          }
          else
          {
            m_buffer.erase();
            m_bufferHeader = EmptyString;
            m_fragmentBuffer.erase();
            m_wsstate = wsStateDisconnecting;
            sendCloseWebSocketFrame( wsStatusProtocolError );
          }
        }
        break;
      }
      case wsStateDisconnecting:
      {
        while( m_buffer.length() > 0 )
        {
          FrameWebSocket frame( m_logInstance, m_flags );
          frame.decode( m_buffer );
//           frame.dump();

          m_buffer.erase( 0, frame.getFrameLen() );

          if( frame.getType() == FrameWebSocket::wsFrameClose )
          {
            m_wsstate = wsStateDisconnected;
            m_state = StateDisconnected;
          }
          else
          {
            m_logInstance.warn( LogAreaClassConnectionWebSocket,
                                "Incorrect frame type received during disconnecting: "
                                  + util::int2string( frame.getType() ) );
          }
        }
        break;
      }
      default:
        m_logInstance.err( LogAreaClassConnectionWebSocket, "WebSocket Unknown state" );
        break;
    }
  }
  
  void ConnectionWebSocket::handleConnect( const ConnectionBase* /*connection*/ )
  {
    m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                       "ConnectionWebSocket::handleConnect state=" + util::int2string( m_state  ) );
    
    if( m_state == StateConnecting )
    {
      m_state = StateConnected;

      if( sendOpeningHandshake() == true )
      {
        m_wsstate = wsStateClientOpeningHandshake;
      }
    }
  }
  
  void ConnectionWebSocket::handleDisconnect( const ConnectionBase* /*connection*/,
                                              ConnectionError reason )
  {
    cleanup();
    
    if( m_handler )
    {
      m_handler->handleDisconnect( this, reason );
    }
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket,
                       "Connection closed: " + util::int2string( reason ) );
  }
  
  long int ConnectionWebSocket::getRandom( unsigned char* buf, int len )
  {
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
    size_t size = len;
    size_t chunk;
    
    while( size > 0 )
    {
      chunk = size > INT_MAX ? INT_MAX : size;
      if( !CryptGenRandom( m_hCryptProv, (DWORD)chunk, buf ) )
      {
        m_logInstance.err( LogAreaClassConnectionWebSocket, "Error when generating random number." );
        return -1;
      }
      buf += chunk;
      size -= chunk;
    }
    
    return len;
#else
    long int n = -1;
    if( m_fd_random < 0 )
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Unknown problem with fd_random." );

    n = read( m_fd_random, reinterpret_cast<char*>( buf ), len );

    if( n < 0 )
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                          "Error when reading from fd_random " + std::string( strerror( errno ) ) );
    
    return n;
#endif // _WIN32
  }
  
  bool ConnectionWebSocket::sendOpeningHandshake()
  {
    if( !m_connection )
      return false;
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Opening Handshake request." );
    
    char hash[16];
    std::string request = "GET " + m_path;
    request += " HTTP/1.1\r\n";
    request += "Host: " + m_server + "\r\n";
    request += "Upgrade: websocket\r\n";
    request += "Connection: Upgrade\r\n";
    request += "Sec-WebSocket-Version: 13\r\n";
    request += "Sec-WebSocket-Protocol: xmpp\r\n";
    
#if defined( _WIN32 ) || defined ( _WIN32_WCE )
    int n = getRandom( (BYTE*)hash, 16 );
#else
    long int n = getRandom( reinterpret_cast<unsigned char*>( hash ), 16 );
#endif
    
    if( n != 16 )
    {
      char l[10];
      sprintf( l, "%ld%c", n, '\0' );
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Incorrect size [" + std::string( l )
                                                            + "]of random key: " + std::string( hash ) );
      return false;
    }
    
    request += "Sec-WebSocket-Key: ";
    m_clientOpeningHandshake = Base64::encode64( hash );
    request += m_clientOpeningHandshake;
    request += "\r\n\r\n";
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Send opening handshake:\n" + request );

    return m_connection->send( request );
  }
  
  bool ConnectionWebSocket::sendOpenXmppStream()
  {
    if( !m_connection )
      return false;
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Opening XMPP stream request." );
    
    std::string request = "<open xmlns='urn:ietf:params:xml:ns:xmpp-framing' to='";
    request += m_server;
    request += "' version='1.0' />";
    
    FrameWebSocket frame( m_logInstance, request, FrameWebSocket::wsFrameText, m_bMasked, m_flags );
    
    frame.encode();

    return sendFrame( frame );
  }
  
  bool ConnectionWebSocket::sendStartXmppStream()
  {
    if( !m_connection )
      return false;
    
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Start XMPP stream request." );
    
    std::string request = "<?xml version='1.0' ?>"
      "<stream:stream xmlns:stream='http://etherx.jabber.org/streams'"
      " xmlns='" + XMLNS_CLIENT + "' version='" + XMPP_STREAM_VERSION_MAJOR
      + "." + XMPP_STREAM_VERSION_MINOR + "' to='" + m_server +  "' xml:lang='en'>\r\n";
    
    FrameWebSocket frame( m_logInstance, request, FrameWebSocket::wsFrameText, m_bMasked, m_flags );
    
    frame.encode();
    
    return sendFrame( frame );
  }
  
bool ConnectionWebSocket::sendCloseXmppStream( gloox::ConnectionError err )
  {
    if( !m_connection )
      return false;
    
    if( m_state != StateDisconnected )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Close XMPP stream request." );
      
      std::string request = "<close xmlns='urn:ietf:params:xml:ns:xmpp-framing' />";
      
      FrameWebSocket frame( m_logInstance, request, FrameWebSocket::wsFrameText, m_bMasked, m_flags );
      
      frame.encode();
      
      if( sendFrame( frame ) )
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "XMPP disconnect request sent." );

    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                          "Disconnecting from server in a non-graceful fashion." );
    }
    
    if( m_handler )
      m_handler->handleDisconnect( this, err );
    
    return true;
  }
  
  bool ConnectionWebSocket::sendPongWebSocketFrame( std::string body )
  {
    if( !m_connection )
      return false;
    
    if( m_state != StateDisconnected )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Pong Frame request." );
      
      FrameWebSocket frame( m_logInstance, body, FrameWebSocket::wsFramePong, m_bMasked, m_flags );
      
      frame.encode();

      if( !sendFrame(frame) )
      {
        return false;
      }
    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                          "Pong Frame can't be sent: connection not established." );
    }
    
    return true;
  }
  
  bool ConnectionWebSocket::sendPingWebSocketFrame( std::string body )
  {
    if( !m_connection )
      return false;
    
    if( m_state != StateDisconnected )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Ping Frame request." );
      
      const std::string fakeBody( "something" );
      FrameWebSocket frame( m_logInstance, ( body.length() ? body : fakeBody ),
                            FrameWebSocket::wsFramePing, m_bMasked, m_flags );
      
      frame.encode();
      
      if( !sendFrame(frame) )
      {
        return false;
      }
    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                          "Ping Frame can't be sent: connection not established." );
    }
    
    return true;
  }
  
  bool ConnectionWebSocket::sendCloseWebSocketFrame( const WebSocketStatusCode status, std::string reason )
  {
    if( !m_connection )
      return false;
    
    if( m_state != StateDisconnected )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "Sending Close Frame request." );
      
      FrameWebSocket frame( m_logInstance, reason, FrameWebSocket::wsFrameClose, m_bMasked, m_flags );
      
      frame.encode( false, status );
      
      if( sendFrame( frame ) )
      {
        m_wsstate = wsStateDisconnecting;
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "XMPP disconnect request sent." );
      }
    }
    else
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket,
                          "Disconnecting from server in a non-graceful fashion." );
    }
    
    if( m_handler )
      m_handler->handleDisconnect( this, ConnUserDisconnected );
    
    return true;
  }
  
  bool ConnectionWebSocket::sendFrame( ConnectionWebSocket::FrameWebSocket& frame )
  {
    return m_connection->send( reinterpret_cast<char*>( frame.getEncodedBuffer() ), frame.getFrameLen() );
  }
  
  void ConnectionWebSocket::handleTag( Tag* tag )
  {
    m_logInstance.dbg( LogAreaClassConnectionWebSocket, "handleTag(): " + tag->xml() );
    /****
     * The XML stream "headers" (the <open/> and <close/> elements) MUST be
     * qualified by the namespace 'urn:ietf:params:xml:ns:xmpp-framing' (the
     * "framed stream namespace").  If this rule is violated, the entity
     * that receives the offending stream header MUST close the stream with
     * an error, which MUST be <invalid-namespace> (see Section 4.9.3.10 of
     * [RFC6120]).
     */

    if( !tag )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "handleTag(): Null tag, closing stream." );
      sendCloseXmppStream( ConnStreamClosed );
      return;
    }

    // We only check open & close tags
    if( tag->name().find( "open" ) == std::string::npos && tag->name().find( "close" ) == std::string::npos )
      return;

    if( tag->xmlns() != XMLNS_XMPP_FRAMING )
    {
      m_logInstance.err( LogAreaClassConnectionWebSocket, "handleTag(): invalid xmlns, framing is missing." );

      std::string request = "<stream:error><invalid-namespace xmlns='" + XMLNS_XMPP_STREAM
                                + "'></invalid-namespace></stream:error>";
      send( request );
      sendCloseXmppStream( ConnStreamError );
      return;
    }

    if( tag->name().find( "open" ) != std::string::npos )
    {
      if( ( tag->findAttribute( "from" ) == m_server ) && ( tag->findAttribute( "id" ).length() != 0 ) &&
          ( tag->findAttribute( "version" ) == "1.0") && ( tag->findAttribute( "lang" ).length() != 0 ) )
      {
        m_streamInitiationValidated = true;
        m_logInstance.dbg( LogAreaClassConnectionWebSocket, "handleTag(): m_streamInitiationValidated = true." );
      }
    }
    else if( tag->name().find( "close" ) != std::string::npos )
    {
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "handleTag(): close." );
      m_streamClosureValidated = true;
      m_logInstance.dbg( LogAreaClassConnectionWebSocket, "handle Tag : m_streamClosureValidated = true." );
    }
  }

  bool ConnectionWebSocket::parse( const std::string& data )
  {
    std::string copy = data;
    int i = 0;

    if( ( i = m_parser.feed( copy ) ) >= 0 )
    {
      if( m_wsstate == wsStateClientClosingXmpp )
      {
        if( m_streamClosureValidated )
        {
          m_logInstance.dbg( LogAreaClassConnectionWebSocket, "XMPP session ended." );
        }
      }
      else
      {
        if( m_streamInitiationValidated == false )
        {
          m_fragmentBuffer.erase();
          m_buffer.erase();
          m_bufferHeader = EmptyString;

          std::string error = "parse error (at pos ";
          error += util::int2string( i );
          error += "): ";
          m_logInstance.err( LogAreaClassClientbase, error + copy );
          Tag* e = new Tag( "stream:error" );
          new Tag( e, "restricted-xml", "xmlns", XMLNS_XMPP_STREAM );
          send( e->xml() );

          sendCloseXmppStream( ConnParseError );

          m_wsstate = wsStateClientClosingXmpp;

        }

      }
    }

    return false;
  }

}

#endif // GLOOX_MINIMAL
