/*-
 * Copyright (c) 2017 Jonathan Anderson
 * All rights reserved.
 *
 * This software was developed at Memorial University under the
 * NSERC Discovery (RGPIN-2015-06048) and RDC Ignite (#5404.1822.101) programs.
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

#include "platform/Capsicum.hh"
#include "platform/FileDescriptor.hh"
#include "platform/PosixError.hh"

#include <sys/capsicum.h>

using namespace capsh;


// TODO: drive libpreopen explicitly
File Capsicum::Open(std::string path, bool writable) const
{
	int flags = writable ? O_RDWR : O_RDONLY;
	int fd = openat(AT_FDCWD, path.c_str(), flags);
	if (fd < 0)
		throw PosixError("unable to open '" + path + "'");

	return FileDescriptor::TakeOwnership(fd);
}


bool Capsicum::EnterSandbox() const
{
	if (cap_enter() != 0)
		throw PosixError("failed to enter Capsicum capability mode");

	return true;
}

bool Capsicum::Sandboxed() const
{
	return InCapabilityMode();
}

bool Capsicum::InCapabilityMode()
{
	unsigned int capmode;
	if (cap_getmode(&capmode) != 0) {
		throw PosixError("failed to check Capsicum capability mode");
	}

	return capmode;
}
