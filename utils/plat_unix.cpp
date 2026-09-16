/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023 Source2ZE
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __linux__
#include "module.h"
#include "plat.h"
#include <dlfcn.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include "sys/mman.h"
#include <locale>
#include <elf.h>
#include <link.h>
#include "dbg.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "tier0/memdbgon.h"

struct ModuleInfo
{
	const char* path; // in
	uint8_t* base; // out
	uint size; // out
};

// https://github.com/alliedmodders/sourcemod/blob/master/core/logic/MemoryUtils.cpp#L502-L587
// https://github.com/komashchenko/DynLibUtils/blob/5eb95475170becfcc64fd5d32d14ec2b76dcb6d4/module_linux.cpp#L95
int GetModuleInformation(HINSTANCE hModule, void** base, size_t* length, std::vector<Section>& m_sections)
{

	link_map* lmap;
	if (dlinfo(hModule, RTLD_DI_LINKMAP, &lmap) != 0)
	{
		dlclose(hModule);
		return 1;
	}

	int fd = open(lmap->l_name, O_RDONLY);
	if (fd == -1)
	{
		dlclose(hModule);
		return 2;
	}

	struct stat st;
	if (fstat(fd, &st) == 0)
	{
		void* map = mmap(nullptr, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if (map != MAP_FAILED)
		{
			ElfW(Ehdr)* ehdr = static_cast<ElfW(Ehdr)*>(map);
			ElfW(Shdr)* shdrs = reinterpret_cast<ElfW(Shdr)*>(reinterpret_cast<uintptr_t>(ehdr) + ehdr->e_shoff);
			const char* strTab = reinterpret_cast<const char*>(reinterpret_cast<uintptr_t>(ehdr) + shdrs[ehdr->e_shstrndx].sh_offset);
	
			for (auto i = 0; i < ehdr->e_phnum; ++i)
			{
				ElfW(Phdr)* phdr = reinterpret_cast<ElfW(Phdr)*>(reinterpret_cast<uintptr_t>(ehdr) + ehdr->e_phoff + i * ehdr->e_phentsize);
				if (phdr->p_type == PT_LOAD && phdr->p_flags & PF_X)
				{
					*base = reinterpret_cast<void*>(lmap->l_addr + phdr->p_vaddr);
					*length = phdr->p_filesz;
					break;
				}
			}

			for (auto i = 0; i < ehdr->e_shnum; ++i)
			{
				ElfW(Shdr)* shdr = reinterpret_cast<ElfW(Shdr)*>(reinterpret_cast<uintptr_t>(shdrs) + i * ehdr->e_shentsize);
				if (*(strTab + shdr->sh_name) == '\0')
					continue;

				Section section;
				section.m_szName = strTab + shdr->sh_name;
				section.m_pBase = reinterpret_cast<void*>(lmap->l_addr + shdr->sh_addr);
				section.m_iSize = shdr->sh_size;
				section.m_bExecutable = (shdr->sh_flags & SHF_EXECINSTR) != 0;
				m_sections.push_back(section);
			}

			munmap(map, st.st_size);
		}
	}

	close(fd);

	return 0;
}

#endif