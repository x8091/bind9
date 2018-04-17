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

/*
 * 32 bit Fowler/Noll/Vo FNV-1a hash code with modification for BIND
 */

#include <config.h>       // IWYU pragma: keep

#include <stddef.h>
#include <stdint.h>

#include "isc/once.h"
#include "isc/random.h"
#include "isc/util.h"
#include "isc/types.h"
#include "isc/likely.h"
#include "isc/result.h"
#include "isc/hash.h"     // IWYU pragma: keep

static uint32_t fnv_offset_basis;
static isc_once_t fnv_once = ISC_ONCE_INIT;
static isc_boolean_t fnv_initialized = ISC_FALSE;

static unsigned char maptolower[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
	0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
	0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
	0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
	0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
	0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
	0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
	0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
	0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
	0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
	0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
	0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
	0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
	0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
	0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
	0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
	0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
	0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
	0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
	0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
	0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
	0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

static void
fnv_initialize(void) {
	/*
	 * This function should not leave fnv_offset_basis set to
	 * 0. Also, after this function has been called, if it is called
	 * again, it should not change fnv_offset_basis.
	 */
	while (fnv_offset_basis == 0) {
		fnv_offset_basis = isc_random32();
	}

	fnv_initialized = ISC_TRUE;
}

const void *
isc_hash_get_initializer(void) {
	if (ISC_UNLIKELY(!fnv_initialized))
		RUNTIME_CHECK(isc_once_do(&fnv_once, fnv_initialize) == ISC_R_SUCCESS);

	return (&fnv_offset_basis);
}

void
isc_hash_set_initializer(const void *initializer) {
	REQUIRE(initializer != NULL);

	/*
	 * Ensure that fnv_initialize() is not called after
	 * isc_hash_set_initializer() is called.
	 */
	if (ISC_UNLIKELY(!fnv_initialized))
		RUNTIME_CHECK(isc_once_do(&fnv_once, fnv_initialize) == ISC_R_SUCCESS);

	fnv_offset_basis = *((const unsigned int *) initializer);
}

#define FNV_32_PRIME ((uint32_t)0x01000193)

uint32_t
isc_hash_function(const void *data, size_t length,
		  isc_boolean_t case_sensitive,
		  const uint32_t *previous_hashp)
{
	uint32_t hval;
	const unsigned char *bp;
	const unsigned char *be;

	REQUIRE(length == 0 || data != NULL);

	if (ISC_UNLIKELY(!fnv_initialized)) {
		RUNTIME_CHECK(isc_once_do(&fnv_once, fnv_initialize) == ISC_R_SUCCESS);
	}

	hval = ISC_UNLIKELY(previous_hashp != NULL) ?
		*previous_hashp : fnv_offset_basis;

	if (length == 0) {
		return (hval);
	}

	bp = (const unsigned char *) data;
	be = bp + length;

	/*
	 * Fowler-Noll-Vo FNV-1a hash function.
	 *
	 * NOTE: A random FNV offset basis is used by default to avoid
	 * collision attacks as the hash function is reversible. This
	 * makes the mapping non-deterministic, but the distribution in
	 * the domain is still uniform.
	 */

	if (case_sensitive) {
		while (bp < be) {
			hval ^= *bp++;
			hval *= FNV_32_PRIME;
		}
	} else {
		while (bp < be) {
			hval ^= maptolower[*bp++];
			hval *= FNV_32_PRIME;
		}
	}

	return (hval);
}

uint32_t
isc_hash_function_reverse(const void *data, size_t length,
			  isc_boolean_t case_sensitive,
			  const uint32_t *previous_hashp)
{
	uint32_t hval;
	const unsigned char *bp;
	const unsigned char *be;

	REQUIRE(length == 0 || data != NULL);

	if (ISC_UNLIKELY(!fnv_initialized)) {
		RUNTIME_CHECK(isc_once_do(&fnv_once, fnv_initialize) == ISC_R_SUCCESS);
	}

	hval = ISC_UNLIKELY(previous_hashp != NULL) ?
		*previous_hashp : fnv_offset_basis;

	if (length == 0) {
		return (hval);
	}

	bp = (const unsigned char *) data;
	be = bp + length;

	/*
	 * Fowler-Noll-Vo FNV-1a hash function.
	 *
	 * NOTE: A random FNV offset basis is used by default to avoid
	 * collision attacks as the hash function is reversible. This
	 * makes the mapping non-deterministic, but the distribution in
	 * the domain is still uniform.
	 */

	if (case_sensitive) {
		while (--be >= bp) {
			hval ^= *be;
			hval *= FNV_32_PRIME;
		}
	} else {
		while (--be >= bp) {
			hval ^= maptolower[*be];
			hval *= FNV_32_PRIME;
		}
	}

	return (hval);
}
