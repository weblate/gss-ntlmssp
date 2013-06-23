/*
   Copyright (C) 2013 Simo Sorce <simo@samba.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _NTLM_H_
#define _NTLM_H

/* Negotiate Flags */
#define NTLMSSP_NEGOTIATE_56                        (1 << 31)
#define NTLMSSP_NEGOTIATE_KEY_EXCH                  (1 << 30)
#define NTLMSSP_NEGOTIATE_128                       (1 << 29)
#define UNUSED_R1                                   (1 << 28)
#define UNUSED_R2                                   (1 << 27)
#define UNUSED_R3                                   (1 << 26)
#define NTLMSSP_NEGOTIATE_VERSION                   (1 << 25)
#define UNUSED_R4                                   (1 << 24)
#define NTLMSSP_NEGOTIATE_TARGET_INFO               (1 << 23)
#define NTLMSSP_REQUEST_NON_NT_SESSION_KEY          (1 << 22)
#define UNUSED_R5 /* Davenport: NEGOTIATE_ACCEPT */ (1 << 21)
#define NTLMSSP_NEGOTIATE_IDENTIFY                  (1 << 20)
#define NTLMSSP_NEGOTIATE_EXTENDED_SESSIONSECURITY  (1 << 19)
#define UNUSED_R6 /* Davenport:TARGET_TYPE_SHARE */ (1 << 18)
#define NTLMSSP_TARGET_TYPE_SERVER                  (1 << 17)
#define NTLMSSP_TARGET_TYPE_DOMAIN                  (1 << 16)
#define NTLMSSP_NEGOTIATE_ALWAYS_SIGN               (1 << 15)
#define UNUSED_R7 /* Davenport:LOCAL_CALL */        (1 << 14)
#define NTLMSSP_NEGOTIATE_OEM_WORKSTATION_SUPPLIED  (1 << 13)
#define NTLMSSP_NEGOTIATE_OEM_DOMAIN_SUPPLIED       (1 << 12)
#define NTLMSSP_ANONYMOUS                           (1 << 11)
#define UNUSED_R8                                   (1 << 10)
#define NTLMSSP_NEGOTIATE_NTLM                      (1 << 9)
#define UNUSED_R9                                   (1 << 8)
#define NTLMSSP_NEGOTIATE_LM_KEY                    (1 << 7)
#define NTLMSSP_NEGOTIATE_DATAGRAM                  (1 << 6)
#define NTLMSSP_NEGOTIATE_SEAL                      (1 << 5)
#define NTLMSSP_NEGOTIATE_SIGN                      (1 << 4)
#define UNUSED_R10                                  (1 << 3)
#define NTLMSSP_REQUEST_TARGET                      (1 << 2)
#define NTLMSSP_NEGOTIATE_OEM                       (1 << 1)
#define NTLMSSP_NEGOTIATE_UNICODE                   (1 << 0)

/* (2.2.2.10 VERSION) */
#define WINDOWS_MAJOR_VERSION_5 0x05
#define WINDOWS_MAJOR_VERSION_6 0x06
#define WINDOWS_MINOR_VERSION_0 0x00
#define WINDOWS_MINOR_VERSION_1 0x01
#define WINDOWS_MINOR_VERSION_2 0x02
#define NTLMSSP_REVISION_W2K3 0x0F

#define NTLMSSP_VERSION_MAJOR WINDOWS_MAJOR_VERSION_6
#define NTLMSSP_VERSION_MINOR WINDOWS_MINOR_VERSION_2
#define NTLMSSP_VERSION_BUILD 0
#define NTLMSSP_VERSION_REV NTLMSSP_REVISION_W2K3

enum ntlm_err_code {
    ERR_BASE = 0x4E540000, /* base error space at 'NT00' */
    ERR_DECODE,
    ERR_ENCODE,
};
#define NTLM_ERR_MASK 0x4E54FFFF
#define IS_NTLM_ERR_CODE(x) (((x) & NTLM_ERR_MASK) ? true : false)

#define discard_const(ptr) ((void *)((uintptr_t)(ptr)))
#define safefree(x) do { free(x); x = NULL; } while(0)

struct ntlm_buffer {
    uint8_t *data;
    size_t length;
};

void ntlm_free_buffer_data(struct ntlm_buffer *buf);

uint64_t ntlm_timestamp_now(void);


enum ntlm_role {
    NTLM_CLIENT,
    NTLM_SERVER,
    NTLM_DOMAIN_SERVER,
    NTLM_DOMAIN_CONTROLLER
};

struct ntlm_ctx;

/**
 * @brief           Create a ntlm_ctx initialized to the initial state
 *
 * @param ctx       The returned context
 *
 * @return 0 if successful, an error otherwise
 */
int ntlm_init_ctx(struct ntlm_ctx **ctx);

/**
 * @brief           Frees a ntlm_ctx
 *
 * @param ctx       Pointer to a context to be freed
 *
 * @return 0 if successful, an error otherwise
 * NOTE: even if an error is returned the contetx is freed and NULLed
 */
int ntlm_free_ctx(struct ntlm_ctx **ctx);

/**
 * @brief   A utility function to construct a target_info structure
 *
 * @param ctx                   The ntlm context
 * @param nb_computer_name      The NetBIOS Computer Name
 * @param nb_domain_name        The NetBIOS Domain Name
 * @param dns_computer_name     The DNS Fully Qualified Computer Name
 * @param dns_domain_name       The DNS Fully Qualified Domain Name
 * @param dns_tree_name         The DNS Tree Name
 * @param av_flags              The av flags
 * @param av_timestamp          A 64 bit FILETIME timestamp
 * @param av_single_host        A ntlm_buffer with the single host data
 * @param av_target_name        The target name
 * @param av_cb                 A ntlm_buffer with channel binding data
 * @param target_info           The buffer in which target_info is returned.
 *
 * NOTE: The caller is responsible for free()ing the buffer
 *
 * @return      0 if everyting parses correctly, or an error code
 */
int ntlm_encode_target_info(struct ntlm_ctx *ctx, char *nb_computer_name,
                            char *nb_domain_name, char *dns_computer_name,
                            char *dns_domain_name, char *dns_tree_name,
                            uint32_t *av_flags, uint64_t *av_timestamp,
                            struct ntlm_buffer *av_single_host,
                            char *av_target_name, struct ntlm_buffer *av_cb,
                            struct ntlm_buffer *target_info);


/**
 * @brief   A utility function to parse a target_info structure
 *
 * @param ctx                   The ntlm context
 * @param buffer                A ntlm_buffer containing the info to be parsed
 * @param nb_computer_name      The NetBIOS Computer Name
 * @param nb_domain_name        The NetBIOS Domain Name
 * @param dns_computer_name     The DNS Fully Qualified Computer Name
 * @param dns_domain_name       The DNS Fully Qualified Domain Name
 * @param dns_tree_name         The DNS Tree Name
 * @param av_flags              The av flags
 * @param av_timestamp          A 64 bit FILETIME timestamp
 * @param av_single_host        A ntlm_buffer with the single host data
 * @param av_target_name        The target name
 * @param av_cb                 A ntlm_buffer with channel binding data
 *
 * NOTE: The caller is responsible for free()ing all strings, while the
 *       ntlm_buffer types point directly at data in the provided buffer.
 *
 * @return      0 if everyting parses correctly, or an error code
 */
int ntlm_decode_target_info(struct ntlm_ctx *ctx, struct ntlm_buffer *buffer,
                            char **nb_computer_name, char **nb_domain_name,
                            char **dns_computer_name, char **dns_domain_name,
                            char **dns_tree_name, char **av_target_name,
                            uint32_t *av_flags, uint64_t *av_timestamp,
                            struct ntlm_buffer *av_single_host,
                            struct ntlm_buffer *av_cb);

/**
 * @brief Verifies the message signature is valid and the message
 * in sequence with the expected state
 *
 * @param ctx           The conversation context.
 * @param buffer        A ntlm_buffer containing the raw NTLMSSP packet
 *
 * @return      0 if everyting parses correctly, or an error code
 *
 * NOTE: Always use ntlm_detect_msg_type before calling other functions,
 * so that the signature and message type are checked, and the state is
 * validated.
 */
int ntlm_decode_msg_type(struct ntlm_ctx *ctx,
                         struct ntlm_buffer *buffer,
                         uint32_t *type);

/**
 * @brief This function encodes a NEGTIATE_MESSAGE which is the first message
 * a client will send to a server. It also updates the stage in the context.
 *
 * @param ctx           A fresh ntlm context.
 * @param flags         Requested flags
 * @param domain        Optional Domain Name
 * @param workstation   Optional Workstation Name
 * @param message       A ntlm_buffer containing the returned message
 *
 * NOTE: the caller is responsible for free()ing the message buffer.
 *
 * @return      0 if everyting encodes correctly, or an error code
 */
int ntlm_encode_neg_msg(struct ntlm_ctx *ctx, uint32_t flags,
                        const char *domain, const char *workstation,
                        struct ntlm_buffer *message);

/**
 * @brief This function decodes a NTLMSSP NEGTIATE_MESSAGE.
 *
 * @param ctx           A fresh ntlm context
 * @param buffer        A ntlm_buffer containing the raw NTLMSSP packet
 * @param flags         Returns the flags requested by the client
 * @param domain        Returns the domain provided by the client if any
 * @param workstation   Returns the workstation provided by the client if any
 *
 * @return      0 if everyting parses correctly, or an error code
 *
 */
int ntlm_decode_neg_msg(struct ntlm_ctx *ctx,
                        struct ntlm_buffer *buffer, uint32_t *flags,
                        char **domain, char **workstation);

/**
 * @brief This function encodes a CHALLENGE_MESSAGE which is the first message
 * a server will send to a client. It also updates the stage in the context.
 *
 * @param ctx           The ntlm context
 * @param flags         The challenge flags
 * @param target_name   The target name
 * @param challenge     A 64 bit value with a challenge
 * @param target_info   A buffer containing target_info data
 * @param message       A ntlm_buffer containing the encoded message
 *
 * NOTE: the caller is responsible for free()ing the message buffer
 *
 * @return      0 if everyting encodes correctly, or an error code
 */
int ntlm_encode_chal_msg(struct ntlm_ctx *ctx,
                         uint32_t flags,
                         char *target_name,
                         struct ntlm_buffer *challenge,
                         struct ntlm_buffer *target_info,
                         struct ntlm_buffer *message);


/**
 * @brief This function decodes a NTLMSSP CHALLENGE_MESSAGE.
 *
 * @param ctx           The ntlm context
 * @param buffer        A ntlm_buffer containing the raw NTLMSSP packet
 * @param flags         The challenge flags
 * @param target_name   The target name
 * @param challenge     A 64 bit value with the server challenge
 *                      The caller MUST provide a preallocated buffer of
 *                      appropriate length (8 bytes)
 * @param target_info   A buffer containing returned target_info data
 *
 * @return      0 if everyting encodes correctly, or an error code
 */
int ntlm_decode_chal_msg(struct ntlm_ctx *ctx,
                         struct ntlm_buffer *buffer,
                         uint32_t *flags, char **target_name,
                         struct ntlm_buffer *challenge,
                         struct ntlm_buffer *target_info);


/**
 * @brief This function encodes a AUTHENTICATE_MESSAGE which is the second
 * message a client will send to a serve.
 * It also updates the stage in the context.
 *
 * @param ctx           The ntlm context
 * @param flags         The flags
 * @param lm_chalresp   A LM or LMv2 response
 * @param nt_chalresp   A NTLM or NTLMv2 response
 * @param domain_name   The Domain name
 * @param user_name     The User name
 * @param workstation   The Workstation name
 * @param enc_sess_key  The session key
 * @param mic           A MIC of the messages
 * @param message       A ntlm_buffer containing the encoded message
 *
 * @return      0 if everyting encodes correctly, or an error code
 */
int ntlm_encode_auth_msg(struct ntlm_ctx *ctx,
                         uint32_t flags,
                         struct ntlm_buffer *lm_chalresp,
                         struct ntlm_buffer *nt_chalresp,
                         char *domain_name, char *user_name,
                         char *workstation,
                         struct ntlm_buffer *enc_sess_key,
                         struct ntlm_buffer *mic,
                         struct ntlm_buffer *message);

/**
 * @brief This function decodes a NTLMSSP AUTHENTICATE_MESSAGE.
 *
 * @param ctx           The ntlm context
 * @param buffer        A ntlm_buffer containing the raw NTLMSSP packet
 * @param flags         The negotiated flags
 * @param lm_chalresp   A LM or LMv2 response
 * @param nt_chalresp   A NTLM or NTLMv2 response
 * @param domain_name   The Domain name
 * @param user_name     The User name
 * @param workstation   The Workstation name
 * @param enc_sess_key  The session key
 * @param mic           A MIC of the messages
 *                      Passing a pointer to a mic means the caller has
 *                      previously requested the presence of a MIC field from
 *                      the peer. If a MIC is not returned by the peer the
 *                      secoding will fail. If not MIC ha sbeen previously
 *                      requested set this pointer to NULL.
 *                      The caller must provide a preallocated buffer of
 *                      appropriate length (16 bytes)
 *
 * NOTE: the caller is reponsible for freeing all allocated buffers
 * on success.
 *
 * @return      0 if everyting encodes correctly, or an error code
 */
int ntlm_decode_auth_msg(struct ntlm_ctx *ctx,
                         struct ntlm_buffer *buffer,
                         uint32_t flags,
                         struct ntlm_buffer *lm_chalresp,
                         struct ntlm_buffer *nt_chalresp,
                         char **domain_name, char **user_name,
                         char **workstation,
                         struct ntlm_buffer *enc_sess_key,
                         struct ntlm_buffer *mic);

#endif /* _NTLM_H_ */
