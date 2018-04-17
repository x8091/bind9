/*
 * Copyright (C) Internet Systems Consortium, Inc. ("ISC")
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * See the COPYRIGHT file distributed with this work for additional
 * information regarding copyright ownership.
 */


#ifndef DST_GSSAPI_H
#define DST_GSSAPI_H 1

/*! \file dst/gssapi.h */

#include <stdint.h>

#include <isc/formatcheck.h>
#include <isc/lang.h>
#include <isc/platform.h>
#include <isc/types.h>
#include <dns/types.h>

#ifdef GSSAPI
#ifdef WIN32
/*
 * MSVC does not like macros in #include lines.
 */
#include <gssapi/gssapi.h>
#include <gssapi/gssapi_krb5.h>
#else
#include ISC_PLATFORM_GSSAPIHEADER
#ifdef ISC_PLATFORM_GSSAPI_KRB5_HEADER
#include ISC_PLATFORM_GSSAPI_KRB5_HEADER
#endif
#endif
#ifndef GSS_SPNEGO_MECHANISM
#define GSS_SPNEGO_MECHANISM ((void*)0)
#endif
#endif

ISC_LANG_BEGINDECLS

/***
 *** Types
 ***/

/***
 *** Functions
 ***/

isc_result_t
dst_gssapi_acquirecred(const dns_name_t *name, isc_boolean_t initiate,
		       gss_cred_id_t *cred);
/*
 *	Acquires GSS credentials.
 *
 *	Requires:
 * 	'name' 	    is a valid name, preferably one known by the GSS provider
 * 	'initiate'  indicates whether the credentials are for initiating or
 *		    accepting contexts
 *      'cred'      is a pointer to NULL, which will be allocated with the
 *		    credential handle.  Call dst_gssapi_releasecred to free
 *		    the memory.
 *
 *	Returns:
 *		ISC_R_SUCCESS msg was successfully updated to include the
 *				      query to be sent
 *		other		  an error occurred while building the message
 */

isc_result_t
dst_gssapi_releasecred(gss_cred_id_t *cred);
/*
 *	Releases GSS credentials.  Calling this function does release the
 *  memory allocated for the credential in dst_gssapi_acquirecred()
 *
 *	Requires:
 *      'mctx'  is a valid memory context
 *      'cred'  is a pointer to the credential to be released
 *
 *	Returns:
 *		ISC_R_SUCCESS 	credential was released successfully
 *		other		an error occurred while releaseing
 *				the credential
 */

isc_result_t
dst_gssapi_initctx(const dns_name_t *name, isc_buffer_t *intoken,
		   isc_buffer_t *outtoken, gss_ctx_id_t *gssctx,
		   isc_mem_t *mctx, char **err_message);
/*
 *	Initiates a GSS context.
 *
 *	Requires:
 * 	'name'     is a valid name, preferably one known by the GSS
 * 	provider
 * 	'intoken'  is a token received from the acceptor, or NULL if
 *		   there isn't one
 * 	'outtoken' is a buffer to receive the token generated by
 *		   gss_init_sec_context() to be sent to the acceptor
 *      'context'  is a pointer to a valid gss_ctx_id_t
 *                 (which may have the value GSS_C_NO_CONTEXT)
 *
 *	Returns:
 *		ISC_R_SUCCESS   msg was successfully updated to include the
 * 				query to be sent
 *		other		an error occurred while building the message
 *		*err_message	optional error message
 */

isc_result_t
dst_gssapi_acceptctx(gss_cred_id_t cred,
		     const char *gssapi_keytab,
		     isc_region_t *intoken, isc_buffer_t **outtoken,
		     gss_ctx_id_t *context, dns_name_t *principal,
		     isc_mem_t *mctx);
/*
 *	Accepts a GSS context.
 *
 *	Requires:
 * 	'mctx'     is a valid memory context
 *      'cred'     is the acceptor's valid GSS credential handle
 * 	'intoken'  is a token received from the initiator
 * 	'outtoken' is a pointer a buffer pointer used to return the token
 *		   generated by gss_accept_sec_context() to be sent to the
 *		   initiator
 *      'context'  is a valid pointer to receive the generated context handle.
 *                 On the initial call, it should be a pointer to NULL, which
 *		   will be allocated as a gss_ctx_id_t.  Subsequent calls
 *		   should pass in the handle generated on the first call.
 *		   Call dst_gssapi_releasecred to delete the context and free
 *		   the memory.
 *
 *	Requires:
 *		'outtoken' to != NULL && *outtoken == NULL.
 *
 *	Returns:
 *		ISC_R_SUCCESS   msg was successfully updated to include the
 * 				query to be sent
 *		DNS_R_CONTINUE	transaction still in progress
 *		other 		an error occurred while building the message
 */

isc_result_t
dst_gssapi_deletectx(isc_mem_t *mctx, gss_ctx_id_t *gssctx);
/*
 *	Destroys a GSS context.  This function deletes the context from the GSS
 *  	provider and then frees the memory used by the context pointer.
 *
 *	Requires:
 *      'mctx'    is a valid memory context
 *	'context' is a valid GSS context
 *
 *	Returns:
 *		ISC_R_SUCCESS
 */


void
gss_log(int level, const char *fmt, ...)
ISC_FORMAT_PRINTF(2, 3);
/*
 * Logging function for GSS.
 *
 *  Requires
 *      'level' is the log level to be used, as an integer
 *      'fmt'   is a printf format specifier
 */

char *
gss_error_tostring(uint32_t major, uint32_t minor,
		   char *buf, size_t buflen);
/*
 *	Render a GSS major status/minor status pair into a string
 *
 *	Requires:
 *      'major' is a GSS major status code
 * 	'minor' is a GSS minor status code
 *
 *	Returns:
 *		A string containing the text representation of the error codes.
 *      	Users should copy the string if they wish to keep it.
 */

isc_boolean_t
dst_gssapi_identitymatchesrealmkrb5(const dns_name_t *signer,
				    const dns_name_t *name,
				    const dns_name_t *realm);
/*
 *	Compare a "signer" (in the format of a Kerberos-format Kerberos5
 *	principal: host/example.com@EXAMPLE.COM) to the realm name stored
 *	in "name" (which represents the realm name).
 *
 */

isc_boolean_t
dst_gssapi_identitymatchesrealmms(const dns_name_t *signer,
				  const dns_name_t *name,
				  const dns_name_t *realm);
/*
 *	Compare a "signer" (in the format of a Kerberos-format Kerberos5
 *	principal: host/example.com@EXAMPLE.COM) to the realm name stored
 *	in "name" (which represents the realm name).
 *
 */

ISC_LANG_ENDDECLS

#endif /* DST_GSSAPI_H */
