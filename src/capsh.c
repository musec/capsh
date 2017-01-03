/*-
 * Copyright (c) 2016 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery program (RGPIN-2015-06048).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/capsicum.h>

#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <libpreopen.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern char **environ;

#define RTLD "/libexec/ld-elf.so.1"


int main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Usage:  capsh <command> [<arg> ...]\n");
		return (1);
	}

	int linker = openat(AT_FDCWD, RTLD, O_RDONLY);
	if (linker < 0) {
		err(-1, "unable to open runtime linker '" RTLD "'");
	}

	int libdir = openat(AT_FDCWD, "/lib", O_RDONLY);
	if (libdir < 0) {
		err(-1, "unable to open library directory '/lib'");
	}

	int binary = openat(AT_FDCWD, argv[1], O_RDONLY);
	if (binary < 0) {
		err(-1, "unable to open executable '%s'", argv[1]);
	}

	//struct po_map *map = po_map_create(4);

	if (cap_enter() != 0) {
		err(-1, "failed to enter capability mode");
	}

	char *libdirstr;
	if (asprintf(&libdirstr, "%d", libdir) < 0) {
		err(-1, "failed to create LD_LIBRARY_PATH_FDS string");
	}

	setenv("LD_LIBRARY_PATH_FDS", libdirstr, 1);
	free(libdirstr);

	fldexec(linker, binary, argv + 1, environ);
	err(-1, "failed to execute '%s'", argv[1]);

	return 0;
}
