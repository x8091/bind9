/*
 * Copyright (C) 2000  Internet Software Consortium.
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

#include <isc/entropy.h>
#include <isc/mem.h>
#include <isc/util.h>
#include <isc/string.h>

#include <stdio.h>

static void
hex_dump(char *msg, void *data, unsigned int length) {
        unsigned int len;
	unsigned char *base;

	base = data;

        printf("DUMP of %d bytes:  %s\n", length, msg);
        for (len = 0 ; len < length ; len++) {
                if (len % 16 == 0)
                        printf("\n");
                printf("%02x ", base[len]);
        }
        printf("\n");
}

static void
CHECK(const char *msg, isc_result_t result) {
	if (result != ISC_R_SUCCESS) {
		printf("FAILURE:  %s\n", msg);
		exit(1);
	}
}

int
main(int argc, char **argv) {
	isc_mem_t *mctx;
	unsigned char buffer[1024];
	isc_entropy_t *ent;
	isc_entropysource_t *devrandom;

	UNUSED(argc);
	UNUSED(argv);

	mctx = NULL;
	CHECK("isc_mem_create()",
	      isc_mem_create(0, 0, &mctx));

	ent = NULL;
	CHECK("isc_entropy_create()",
	      isc_entropy_create(mctx, &ent));

	devrandom = NULL;
	CHECK("isc_entropy_createfilesource()",
	      isc_entropy_createfilesource(ent, "/dev/random",
					   0, &devrandom));
	
	return (0);
}
