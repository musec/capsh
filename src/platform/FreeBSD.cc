
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
#include <sstream>

#include <err.h>
#include <fcntl.h>
#include <libpreopen.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include<ucl.h>

using namespace capsh;
using std::string;
using std::vector;

extern char **environ;


std::unique_ptr<Platform> Platform::Current()
{
        return FreeBSD::Create();
}


std::unique_ptr<Platform> FreeBSD::Create()
{
        vector<int> pathdirs;
        std::stringstream ss(getenv("PATH"));
        while (ss.good())
        {
                string path;
                getline(ss, path, ':');

                int fd = openat(AT_FDCWD, path.c_str(), O_RDONLY);
                if (fd >= 0)
                {
                        pathdirs.push_back(fd);
                }
        }

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

        vector<int> libdirs;
        for (const char *dirname : { "/lib", "/usr/lib", "/usr/local/lib" })
        {
                fd = openat(AT_FDCWD, dirname, O_RDONLY);
                if (fd >= 0)
                {
                        libdirs.push_back(fd);
                }
        }

        return std::unique_ptr<Platform>(
                new FreeBSD(std::move(linkers), libdirs, pathdirs));
}


FreeBSD::FreeBSD(LinkerMap linkers, vector<int> libdirs, vector<int> pathdirs)
        : linkers_(std::move(linkers)), libdirs_(libdirs), pathdirs_(pathdirs)
{
        map = po_map_create(4);
}


CommandLine FreeBSD::ParseArgs(int argc, char *argv[]) const
{
        assert(argc > 0);
        vector<const string> args(argv, argv + argc);
        assert(not args.empty());

        int binary = openat(AT_FDCWD, argv[0], O_RDONLY);
        for (int dir : pathdirs_)
        {
                if (binary >= 0)
                {
                        break;
                }

                binary = openat(dir, argv[0], O_RDONLY);
        }

        if (binary < 0)
                throw PosixError("unable to open executable '" + args[0] + "'");

        char conf_file[20];

        strncpy(conf_file, argv[0],strlen(argv[0])+1);
        strncat(conf_file, ".conf",5);

        FILE *conf;
        conf = fopen(conf_file, "r");

        if(conf != NULL)
        {
                struct ucl_parser *parser = NULL;
                ucl_object_t *top = NULL;

                ucl_object_iter_t it_obj_file = NULL, it_cur_file = NULL;
                ucl_object_iter_t it_obj_directory = NULL, it_cur_directory = NULL;
                const ucl_object_t *root_file, *obj_file, *cur_file,*blind_obj;
                const ucl_object_t *root_directory,*obj_directory, *cur_directory;

                parser = ucl_parser_new (0);
                ucl_parser_add_fd(parser, fileno(conf));
                fclose (conf);

                if (ucl_parser_get_error (parser))
                {
                    printf ("Error occurred: %s\n", ucl_parser_get_error (parser));
                }
                else 
                {
                    top = ucl_parser_get_object (parser);
                }

                //Reading all the files from the configuration
                root_file = ucl_object_find_key(top, "file");

                while ((obj_file = ucl_iterate_object (root_file, &it_obj_file, false))) 
                {
                    while ((cur_file = ucl_iterate_object (obj_file, &it_cur_file, true))) 
                    {
                            if(strcmp(ucl_object_key (cur_file), "name") == 0)
                            {
                                po_preopen(map, ucl_object_tostring(cur_file), O_RDONLY);
                            }

                    }
                }

                root_directory = ucl_object_find_key(top, "directory");

                while ((obj_directory = ucl_iterate_object (root_directory, &it_obj_directory, false))) 
                {

                    while ((cur_directory = ucl_iterate_object (obj_directory, &it_cur_directory, true))) 
                    {
                            if(strcmp(ucl_object_key (cur_directory), "name") == 0)
                            {
                                    po_preopen(map, ucl_object_tostring(cur_directory), O_DIRECTORY);
                            }
                    }
                }

                if (parser != NULL) 
                {
                    ucl_parser_free (parser);
                }

                //Reading if blind preopen is allowed in the policy file or not
                int blind;

                blind_obj = ucl_object_find_key(top,"blind");
                blind = (int)ucl_object_toint(blind_obj);

                if(blind == 1)
                {
                    char *path;
                    int i=1;
                    
                    for (i=1; i <= argc; i++)
                    {
                    //Try to preopen all the arguments given.

                            path = argv[i];

                            if (path == NULL || (*path == '-'))
                            {
                                    continue;
                            }

                            else
                            {
                                    po_preopen(map, path, O_RDONLY);
                            }
                    }
                }
        }

        return CommandLine(File(FileDescriptor::TakeOwnership(binary)), args);
}


void FreeBSD::Execute(const CommandLine& c) const
{
        auto& binary = dynamic_cast<const FileDescriptor&>(*c.executable());
        const FileDescriptor& linker = getLinkerFor(binary);

        // Build arguments vector: rtld -f <FD> -- <binary> <binary args>
        vector<char*> argv
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

        int shmfd = po_pack(map);

    assert(shmfd != -1);

    if (setenv("SHARED_MEMORYFD", std::to_string(shmfd).c_str() ,1)){
            err(-1, "SHARED_MEMORYFD not set");
            }

    //setting the close-on-exec flag to 0
    fcntl(shmfd, F_SETFD, 0);

    setenv("LD_PRELOAD", std::string("libpreopen.so").c_str() ,1);

        // And... go!
        fexecve(linker.borrow(), argv.data(), environ);
        throw PosixError("error in fexecve()");
}


const FileDescriptor& FreeBSD::getLinkerFor(const FileDescriptor&) const
{
        // TODO: ELF parsing, etc.
        auto l = linkers_.find("");
        if (l == linkers_.end())
                throw OSError("no viable linker found");

        return dynamic_cast<const FileDescriptor&>(*l->second);
}

