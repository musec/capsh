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

#ifndef PLATFORM_HH
#define PLATFORM_HH

#include "File.hh"
#include "OSError.hh"

#include <memory>

namespace capsh {

class CommandLine;

/**
 * A description of a platform's mechanisms for implementing sandboxing and
 * executing binaries.
 */
class Platform
{
public:
	//! Construct a representation of the currently-executing platform.
	static std::unique_ptr<Platform> Current();

	virtual ~Platform();

	//! Open a file by absolute path name.
	virtual File Open(std::string path, bool writable = false) const = 0;

	/**
	 * Parse a command-line argument, resolving platform-specific details
	 * such as looking up executable files in the current PATH.
	 */
	virtual CommandLine ParseArgs(int argc, char *argv[]) const = 0;

	/**
	 * Execute a command using platform-appropriate mechanisms to pass
	 * arguments and environment variables to the new process.
	 *
	 * This method should never return: it must either successfully switch
	 * to the execution of the new process or else throw an exception.
	 *
	 * @throw    OSError on failure to execte
	 */
	virtual void Execute(const CommandLine&) const = 0;

	/**
	 * Enter a sandbox that prevents access to global namespaces.
	 *
	 * @throw    OSError if unable to create/enter sandbox
	 */
	virtual bool EnterSandbox() const = 0;

	/**
	 * Is the current process already in a sandbox?
	 *
	 * @throw    OSError if unable to query current sandbox state
	 */
	virtual bool Sandboxed() const = 0;
};

} // namespace capsh

#endif // !defined(PLATFORM_HH)
