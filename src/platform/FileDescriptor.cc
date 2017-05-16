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

#include "platform/FileDescriptor.hh"

#include <cassert>
#include <string>

#include <unistd.h>

using namespace capsh;


capsh::File FileDescriptor::TakeOwnership(int fd)
{
	return File(std::shared_ptr<FileDescriptor>(new FileDescriptor(fd)));
}

FileDescriptor::FileDescriptor(int fd)
	: fd_(fd)
{
	assert(fd >= 0);
}

FileDescriptor::FileDescriptor(FileDescriptor&& orig)
	: fd_(orig.fd_)
{
	orig.fd_ = -1;
}

FileDescriptor::~FileDescriptor()
{
	if (fd_ >= 0)
	{
		close(fd_);
	}
}

std::string FileDescriptor::str() const
{
	return "FD " + std::to_string(fd_);
}
