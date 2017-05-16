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

#ifndef PLATFORM_FREEBSD_HH
#define PLATFORM_FREEBSD_HH

#include "platform/Capsicum.hh"
#include "File.hh"

#include <memory>
#include <unordered_map>
#include <vector>

namespace capsh {

class FreeBSD : public Capsicum
{
public:
	static std::unique_ptr<Platform> Create();

	CommandLine ParseArgs(int argc, char *argv[]) const override;

	void Execute(const CommandLine&) const override;

private:
	// A mapping from ABIs (internal numbering) to linker file descriptors.
	using LinkerMap = std::unordered_map<std::string, File>;

	FreeBSD(LinkerMap linkers, std::vector<int> libdirs);

	const FileDescriptor& getLinkerFor(const FileDescriptor&) const;

	const LinkerMap linkers_;
	const std::vector<int> libdirs_;
};

} // namespace capsh

#endif // !defined(PLATFORM_FREEBSD_HH)
