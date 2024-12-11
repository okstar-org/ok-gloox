/*
  Copyright (c) 2013-2019 by Jakob Schröter <js@camaya.net>
  This file is part of the gloox library. http://camaya.net/gloox

  This software is distributed under a license. The full license
  agreement can be found in the file LICENSE in this distribution.
  This software may not be copied, modified, sold or distributed
  other than expressed in the named license agreement.

  This software is distributed without any warranty.
*/


#ifndef JINGLEICEUDP_H__
#define JINGLEICEUDP_H__

#include "jingleplugin.h"

#include <string>
#include <list>

namespace gloox {

    class Tag;

    namespace Jingle {
        class GLOOX_API SignalingWebRTCDatachannels {
        private:
            /* data */
            int m_port;
            int m_bit;
            std::string m_name;

        public:
            SignalingWebRTCDatachannels();

            SignalingWebRTCDatachannels(const int &port,
                                        const int &bit,
                                        const std::string &name);
        };

        /**
         * @brief An abstraction of the signaling part of Jingle ICE-UDP Transport Method (@xep{0176}).
         *
         * XEP Version: 1.0
         *
         * @author Jakob Schröter <js@camaya.net>
         * @since 1.0.7
         */
        class GLOOX_API ICEUDP : public Plugin {
        public:
            /**
             * Describes the candidate type.
             */
            enum Type {
                Host,                     /**< A host candidate. */
                PeerReflexive,            /**< A peer reflexive candidate. */
                Relayed,                  /**< A relayed candidate. */
                ServerReflexive           /**< A server reflexive candidate. */
            };

            struct Dtls {
                std::string hash;
                std::string setup;
                std::string fingerprint;
            };
            typedef std::list<Dtls> DtlsList;

            struct Sctp {
                std::string protocol;
                uint32_t port = 0;
                uint32_t streams = 0;
            };
            typedef std::list<Sctp> SctpList;


            /**
             * Describes a single transport candidate.
             */
            struct Candidate {
                int component;    /**< A Component ID as defined in ICE-CORE. */
                std::string foundation;   /**< A Foundation as defined in ICE-CORE.*/
                uint32_t generation;   /**< An index, starting at 0, that enables the parties to keep track of
                                         updates to the candidate throughout the life of the session. */
                std::string id;           /**< A unique identifier for the candidate. */
                std::string ip;           /**< The IP address for the candidate transport mechanism. */
                uint32_t network;      /**< An index, starting at 0, referencing which network this candidate is on for a given peer. */
                uint32_t port;                 /**< The port at the candidate IP address. */
                uint32_t priority;             /**< A Priority as defined in ICE-CORE. */
                std::string protocol;     /**< The protocol to be used. Should be @b udp. */
                std::string tcptype;
                std::string rel_addr;     /**< A related address as defined in ICE-CORE. */
                uint32_t rel_port;             /**< A related port as defined in ICE-CORE. */
                Type type;                /**< A Candidate Type as defined in ICE-CORE. */
            };

            /** A list of transport candidates. */
            typedef std::list<Candidate> CandidateList;

            /**
             * Constructs a new instance.
             * @param pwd The @c pwd value.
             * @param ufrag The @c ufrag value.
             * @param candidates A list of connection candidates.
             */
            ICEUDP(const std::string &pwd, const std::string &ufrag, const CandidateList &candidates);

            /**
             * Constructs a new instance from the given tag.
             * @param tag The Tag to parse.
             */
            ICEUDP(const Tag *tag = 0);

            /**
             * Virtual destructor.
             */
            virtual ~ICEUDP() {}

            /**
             * Returns the @c pwd value.
             * @return The @c pwd value.
             */
            const std::string &pwd() const { return m_pwd; }

            /**
             * Returns the @c ufrag value.
             * @return The @c ufrag value.
             */
            const std::string &ufrag() const { return m_ufrag; }

            /**
             * Returns the list of connection candidates.
             * @return The list of connection candidates.
             */
            const CandidateList &candidates() const { return m_candidates; }

            const Dtls& dtls() const { return m_dtls; }
            void setDtls(const Dtls &);

            const Sctp& sctp() const {return m_sctp;}
            void setSctp(const Sctp& sctp) ;

            // reimplemented from Plugin
            virtual const StringList features() const;

            // reimplemented from Plugin
            virtual const std::string &filterString() const;

            // reimplemented from Plugin
            virtual Tag *tag() const;

            // reimplemented from Plugin
            virtual Plugin *newInstance(const Tag *tag) const;

            // reimplemented from Plugin
            virtual Plugin *clone() const {
                return new ICEUDP(*this);
            }

            virtual const SignalingWebRTCDatachannels &signal_webrtc_datachannels() const;

            void setSignalingWebRTCDatachannels(const SignalingWebRTCDatachannels &_signalingWebRTCDatachannels);

        private:
            std::string m_pwd;
            std::string m_ufrag;
            CandidateList m_candidates;
            SignalingWebRTCDatachannels signalingWebRTCDatachannels;
            Dtls m_dtls;
            Sctp m_sctp;
        };

    }

}

#endif // JINGLEICEUDP_H__
