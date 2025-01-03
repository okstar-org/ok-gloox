/*
  Copyright (c) 2005-2023 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/

#ifdef ENABLE_VLD
#include <vld.h>
#endif

#include "gloox.h"

namespace gloox
{

  const std::string XMLNS_CLIENT            = "jabber:client";
  const std::string XMLNS_COMPONENT_ACCEPT  = "jabber:component:accept";
  const std::string XMLNS_COMPONENT_CONNECT = "jabber:component:connect";

  const std::string XMLNS_DISCO_INFO        = "http://jabber.org/protocol/disco#info";
  const std::string XMLNS_DISCO_ITEMS       = "http://jabber.org/protocol/disco#items";
  const std::string XMLNS_DISCO_PUBLISH     = "http://jabber.org/protocol/disco#publish";
  const std::string XMLNS_ADHOC_COMMANDS    = "http://jabber.org/protocol/commands";
  const std::string XMLNS_COMPRESSION       = "http://jabber.org/protocol/compress";

  const std::string XMLNS_OFFLINE           = "http://jabber.org/protocol/offline";
  const std::string XMLNS_CHAT_STATES       = "http://jabber.org/protocol/chatstates";
  const std::string XMLNS_AMP               = "http://jabber.org/protocol/amp";
  const std::string XMLNS_IBB               = "http://jabber.org/protocol/ibb";
  const std::string XMLNS_FEATURE_NEG       = "http://jabber.org/protocol/feature-neg";

  const std::string XMLNS_CHATNEG           = "http://jabber.org/protocol/chatneg";
  const std::string XMLNS_XHTML_IM          = "http://jabber.org/protocol/xhtml-im";
  const std::string XMLNS_DELAY             = "urn:xmpp:delay";
  const std::string XMLNS_ROSTER            = "jabber:iq:roster";
  const std::string XMLNS_VERSION           = "jabber:iq:version";

  const std::string XMLNS_REGISTER          = "jabber:iq:register";
  const std::string XMLNS_PRIVACY           = "jabber:iq:privacy";
  const std::string XMLNS_AUTH              = "jabber:iq:auth";
  const std::string XMLNS_PRIVATE_XML       = "jabber:iq:private";
  const std::string XMLNS_LAST              = "jabber:iq:last";

  const std::string XMLNS_SEARCH            = "jabber:iq:search";
  const std::string XMLNS_IQ_OOB            = "jabber:iq:oob";
  const std::string XMLNS_X_DATA            = "jabber:x:data";
  const std::string XMLNS_X_EVENT           = "jabber:x:event";
  const std::string XMLNS_X_OOB             = "jabber:x:oob";

  const std::string XMLNS_X_DELAY           = "jabber:x:delay";
  const std::string XMLNS_X_GPGSIGNED       = "jabber:x:signed";
  const std::string XMLNS_X_GPGENCRYPTED    = "jabber:x:encrypted";
  const std::string XMLNS_VCARD_TEMP        = "vcard-temp";
  const std::string XMLNS_X_VCARD_UPDATE    = "vcard-temp:x:update";

  const std::string XMLNS_BOOKMARKS         = "storage:bookmarks";
  const std::string XMLNS_BOOKMARKS2        = "urn:xmpp:bookmarks:1";
  const std::string XMLNS_BOOKMARKS2_COMPAT = XMLNS_BOOKMARKS2+ "#compat";
  const std::string XMLNS_BOOKMARKS2_NOTIFY = XMLNS_BOOKMARKS2+ "+notify";
  const std::string XMLNS_ANNOTATIONS       = "storage:rosternotes";
  const std::string XMLNS_ROSTER_DELIMITER  = "roster:delimiter";
  const std::string XMLNS_XMPP_PING         = "urn:xmpp:ping";
  const std::string XMLNS_SI                = "http://jabber.org/protocol/si";

  const std::string XMLNS_SI_FT             = "http://jabber.org/protocol/si/profile/file-transfer";
  const std::string XMLNS_BYTESTREAMS       = "http://jabber.org/protocol/bytestreams";

  const std::string XMLNS_X_CONFERENCE      = "jabber:x:conference";
  const std::string XMLNS_MUC               = "http://jabber.org/protocol/muc";
  const std::string XMLNS_MUC_USER          = "http://jabber.org/protocol/muc#user";
  const std::string XMLNS_MUC_ADMIN         = "http://jabber.org/protocol/muc#admin";

  const std::string XMLNS_MUC_UNIQUE        = "http://jabber.org/protocol/muc#unique";
  const std::string XMLNS_MUC_OWNER         = "http://jabber.org/protocol/muc#owner";
  const std::string XMLNS_MUC_ROOMINFO      = "http://jabber.org/protocol/muc#roominfo";
  const std::string XMLNS_MUC_ROOMS         = "http://jabber.org/protocol/muc#rooms";
  const std::string XMLNS_MUC_REQUEST       = "http://jabber.org/protocol/muc#request";

  const std::string XMLNS_PUBSUB            = "http://jabber.org/protocol/pubsub";
  const std::string XMLNS_PUBSUB_ERRORS     = "http://jabber.org/protocol/pubsub#errors";
  const std::string XMLNS_PUBSUB_EVENT      = "http://jabber.org/protocol/pubsub#event";
  const std::string XMLNS_PUBSUB_OWNER      = "http://jabber.org/protocol/pubsub#owner";
  const std::string XMLNS_PUBSUB_PUBLISH_OPTIONS    = "http://jabber.org/protocol/pubsub#publish-options";
  const std::string XMLNS_PUBSUB_AUTO_SUBSCRIBE     = "http://jabber.org/protocol/pubsub#auto-subscribe";
  const std::string XMLNS_PUBSUB_AUTO_CREATE        = "http://jabber.org/protocol/pubsub#auto-create";
  const std::string XMLNS_PUBSUB_NOTIFICATIONS      = "http://jabber.org/protocol/pubsub#presence-notifications";

  const std::string XMLNS_CAPS              = "http://jabber.org/protocol/caps";

  const std::string XMLNS_FT_FASTMODE       = "http://affinix.com/jabber/stream";
  const std::string XMLNS_STREAM            = "http://etherx.jabber.org/streams";
  const std::string XMLNS_XMPP_STREAM       = "urn:ietf:params:xml:ns:xmpp-streams";
  const std::string XMLNS_XMPP_STANZAS      = "urn:ietf:params:xml:ns:xmpp-stanzas";
  const std::string XMLNS_STREAM_TLS        = "urn:ietf:params:xml:ns:xmpp-tls";

  const std::string XMLNS_STREAM_SASL       = "urn:ietf:params:xml:ns:xmpp-sasl";
  const std::string XMLNS_STREAM_BIND       = "urn:ietf:params:xml:ns:xmpp-bind";
  const std::string XMLNS_STREAM_SESSION    = "urn:ietf:params:xml:ns:xmpp-session";
  const std::string XMLNS_STREAM_IQAUTH     = "http://jabber.org/features/iq-auth";
  const std::string XMLNS_STREAM_IQREGISTER = "http://jabber.org/features/iq-register";

  const std::string XMLNS_STREAM_COMPRESS   = "http://jabber.org/features/compress";
  const std::string XMLNS_HTTPBIND          = "http://jabber.org/protocol/httpbind";
  const std::string XMLNS_ADDRESSES         = "http://jabber.org/protocol/address";

  const std::string XMLNS_XMPP_BOSH         = "urn:xmpp:xbosh";
  const std::string XMLNS_RECEIPTS          = "urn:xmpp:receipts";
  const std::string XMLNS_NICKNAME          = "http://jabber.org/protocol/nick";

  const std::string XMLNS_JINGLE            = "urn:xmpp:jingle:1";
  const std::string XMLNS_JINGLE_ERRORS     = "urn:xmpp:jingle:errors:1";
  const std::string XMLNS_JINGLE_MESSAGE    = "urn:xmpp:jingle-message:0";

  const std::string XMLNS_JINGLE_ICE_UDP    = "urn:xmpp:jingle:transports:ice-udp:1";
  const std::string XMLNS_JINGLE_IBB        = "urn:xmpp:jingle:transports:ibb:1";

  const std::string XMLNS_JINGLE_FILE_TRANSFER = "urn:xmpp:jingle:apps:file-transfer:3";
  const std::string XMLNS_JINGLE_FILE_TRANSFER4 = "urn:xmpp:jingle:apps:file-transfer:4";
  const std::string XMLNS_JINGLE_FILE_TRANSFER5 = "urn:xmpp:jingle:apps:file-transfer:5";
  const std::string XMLNS_JINGLE_FILE_TRANSFER_MULTI = "urn:xmpp:jingle:apps:file-transfer:multi";

  const std::string XMLNS_JINGLE_APPS_GROUP      = "urn:xmpp:jingle:apps:grouping:0";
  const std::string XMLNS_JINGLE_APPS_RTP  = "urn:xmpp:jingle:apps:rtp:1";
  const std::string XMLNS_JINGLE_APPS_DTLS = "urn:xmpp:jingle:apps:dtls:0";
  const std::string XMLNS_JINGLE_APPS_DTLS_SCTP = "urn:xmpp:jingle:transports:dtls-sctp:1";
  const std::string XMLNS_JINGLE_APPS_RTP_FB = "urn:xmpp:jingle:apps:rtp:rtcp-fb:0";
  const std::string XMLNS_JINGLE_APPS_RTP_HDREXT = "urn:xmpp:jingle:apps:rtp:rtp-hdrext:0";
  const std::string XMLNS_JINGLE_APPS_RTP_SSMA = "urn:xmpp:jingle:apps:rtp:ssma:0";

    const std::string XMLNS_JINGLE_FEATURE_AUDIO =
        "urn:xmpp:jingle:apps:rtp:audio";
    const std::string XMLNS_JINGLE_FEATURE_VIDEO =
        "urn:xmpp:jingle:apps:rtp:video";

  const std::string XMLNS_SHIM              = "http://jabber.org/protocol/shim";
  const std::string XMLNS_ATTENTION         = "urn:xmpp:attention:0";
  const std::string XMLNS_STREAM_MANAGEMENT = "urn:xmpp:sm:3";
  const std::string XMLNS_STANZA_FORWARDING = "urn:xmpp:forward:0";
  const std::string XMLNS_MESSAGE_CARBONS   = "urn:xmpp:carbons:2";

  const std::string XMLNS_HASHES            = "urn:xmpp:hashes:1";
  const std::string XMLNS_IODATA            = "urn:xmpp:tmp:io-data";
  const std::string XMLNS_ROSTER_X          = "http://jabber.org/protocol/rosterx";
  const std::string XMLNS_CLIENT_STATE_INDICATION = "urn:xmpp:csi:0";
  const std::string XMLNS_XMPP_FRAMING      = "urn:ietf:params:xml:ns:xmpp-framing";

  const std::string XMLNS_CHAT_MARKERS      = "urn:xmpp:chat-markers:0";
  const std::string XMLNS_MESSAGE_MARKUP    = "urn:xmpp:markup:0";
  const std::string XMLNS_REFERENCES        = "urn:xmpp:references:0";
  const std::string XMLNS_HINTS             = "urn:xmpp:hints";
  const std::string XMLNS_SXE               = "urn:xmpp:sxe:0";

  const std::string XMLNS_BOB               = "urn:xmpp:bob";
  const std::string XMLNS_DATAFORM_MEDIA    = "urn:xmpp:media-element";
  const std::string XMLNS_AVATAR            = "urn:xmpp:avatar:data";
  const std::string XMLNS_META_AVATAR       = "urn:xmpp:avatar:metadata";

  const std::string XMPP_STREAM_VERSION_MAJOR = "1";
  const std::string XMPP_STREAM_VERSION_MINOR = "0";
  const std::string GLOOX_VERSION           = "1.1-svn";
  const std::string GLOOX_CAPS_NODE         = "http://camaya.net/gloox";

  const std::string XMLNS = "xmlns";
  const std::string TYPE = "type";
  const std::string EmptyString = "";

  const std::string XMLNS_JITSI_FOCUS           = "http://jitsi.org/protocol/focus";
  const std::string XMLNS_JITSI_MEET            = "https://jitsi.org/jitsi-meet";
  const std::string XMLNS_JIT_MEET              = "http://jitsi.org/jitmeet";
  const std::string JITSI_AUDIO_MUTED           = "http://jitsi.org/jitmeet/audio";
  const std::string JITSI_VIDEO_MUTED           = "http://jitsi.org/jitmeet/video";
  const std::string JITSI_CONFERENCE            = "http://jitsi.org/protocol/focus";
  const std::string JITSI_DEVICES               = "devices";
  const std::string JITSI_VIDEO_TYPE            = "video_type";
  const std::string JITSI_RAISED_HAND           = "raised_hand";
  const std::string JITSI_USER_AGENT            = "user_agent";

  const std::string XMLNS_EXTERNAL_SERVICE_DISCOVERY = "urn:xmpp:extdisco:2";


}

const char* gloox_version()
{
  return gloox::GLOOX_VERSION.c_str();
}
