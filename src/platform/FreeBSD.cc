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
#include "platform/FreeBSD.hh"
#include "CommandLine.hh"
#include "UserError.hh"

#include <cassert>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

using namespace capsh;
using std::string;

extern char **environ;


std::unique_ptr<Platform> Platform::Current()
{
	return FreeBSD::Create();
}


std::unique_ptr<Platform> FreeBSD::Create()
{
	static const char *ENVVAR_NAME = "CAPSH_DEFAULT_LINKER";
	LinkerMap linkers;

	// First find whatever linkers we can find in the global filesystem
	// namespace (or its libpreopen-proxied version):
	int fd = openat(AT_FDCWD, "/libexec/ld-elf.so.1", O_RDONLY);
	if (fd >= 0)
	{
		linkers.emplace("elf", FileDescriptor::TakeOwnership(fd));
	}

	fd = openat(AT_FDCWD, "/libexec/ld-elf32.so.1", O_RDONLY);
	if (fd >= 0)
	{
		linkers.emplace("elf32", FileDescriptor::TakeOwnership(fd));
	}

	// Then set the default linker to something explicitly-specified if
	// there is such a thing or the ELF linker otherwise.
	const char *env = getenv(ENVVAR_NAME);
	if (env != NULL)
	{
		char *end;
		fd = static_cast<int>(strtol(env, &end, 0));
		if (*end != '\0')
			throw UserError(
				string("invalid file descriptor: '")
					+ env + "' is not a number");

		// Default linker:
		linkers.emplace("", FileDescriptor::TakeOwnership(fd));
	}
	else
	{
		auto l = linkers.find("elf");
		if (l == linkers.end())
		{
			throw OSError("no viable linkers found");
		}

		linkers.emplace("", l->second);
	}

	std::vector<int> libdirs;
	for (const char *dirname : { "/lib", "/usr/lib", "/usr/local/lib" })
	{
		fd = openat(AT_FDCWD, dirname, O_RDONLY);
		if (fd >= 0)
		{
			libdirs.push_back(fd);
		}
	}

	return std::unique_ptr<Platform>(
		new FreeBSD(std::move(linkers), libdirs));
}


FreeBSD::FreeBSD(LinkerMap linkers, std::vector<int> libdirs)
	: linkers_(std::move(linkers)), libdirs_(libdirs)
{
}


CommandLine FreeBSD::ParseArgs(int argc, char *argv[]) const
{
	assert(argc > 0);
	std::vector<const string> args(argv, argv + argc);
	assert(not args.empty());

	int fd = openat(AT_FDCWD, argv[0], O_RDONLY);
	if (fd < 0)
		throw PosixError("unable to open executable '" + args[0] + "'");

	return CommandLine(File(FileDescriptor::TakeOwnership(fd)), args);
}


void FreeBSD::Execute(const CommandLine& c) const
{
	auto& binary = dynamic_cast<const FileDescriptor&>(*c.executable());
	const FileDescriptor& linker = getLinkerFor(binary);

	// Build arguments vector: rtld -f <FD> -- <binary> <binary args>
	std::vector<char*> argv
	{
		strdup("rtld"),
		strdup("-f"),
		strdup(std::to_string(binary.borrow()).c_str()),
		strdup("--"),
	};

	for (const string& s : c.arguments())
	{
		argv.push_back(strdup(s.c_str()));
	}
	argv.push_back(NULL);

	// Add a colon-separated list of library directories to the environment.
	string libs;
	const size_t libcount = libdirs_.size();
	for (size_t i = 0; i < libcount; i++)
	{
		libs += std::to_string(libdirs_[i]);
		if (i < (libcount - 1))
		{
			libs += ":";
		}
	}
	setenv("LD_LIBRARY_PATH_FDS", libs.c_str(), 1);

	// And... go!
	fexecve(linker.borrow(), argv.data(), environ);
	throw PosixError("error in fexecve()");
}


const FileDescriptor& FreeBSD::getLinkerFor(const FileDescriptor& binary) const
{
	// TODO: ELF parsing, etc.
	auto l = linkers_.find("");
	if (l == linkers_.end())
		throw OSError("no viable linker found");

	return dynamic_cast<const FileDescriptor&>(*l->second);
}
