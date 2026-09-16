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

#pragma once
#include "dbg.h"
#include "interface.h"
#include "strtools.h"
#include "plat.h"

#ifdef _WIN32
#include <Psapi.h>
#endif

void Message(const char *, ...);
void Panic(const char *, ...);

enum SigError
{
	SIG_OK,
	SIG_NOT_FOUND,
	SIG_FOUND_MULTIPLE,
};

class CModule
{
public:
	CModule(const char *path, const char *module) :
		m_pszModule(module), m_pszPath(path)
	{
		char szModule[MAX_PATH];

		V_snprintf(szModule, MAX_PATH, "%s%s%s%s%s", Plat_GetGameDirectory(), path, MODULE_PREFIX, m_pszModule, MODULE_EXT);

		m_hModule = dlmount(szModule);

		if (!m_hModule)
		{
			Panic("Could not find %s\n", szModule);
			return;
		}

#ifdef _WIN32
		MODULEINFO m_hModuleInfo = {};
		if (!GetModuleInformation(GetCurrentProcess(), m_hModule, &m_hModuleInfo, sizeof(m_hModuleInfo)))
		{
			Panic("Failed to get module info for %s, error %lu\n", szModule, GetLastError());
			return;
		}

		m_base = (void *)m_hModuleInfo.lpBaseOfDll;
		m_size = m_hModuleInfo.SizeOfImage;
		InitializeSections();
#else
		if (int e = GetModuleInformation(m_hModule, &m_base, &m_size, m_sections))
		{
			Panic("Failed to get module info for %s, error %d\n", szModule, e);
			m_hModule = nullptr;
			m_base = nullptr;
			m_size = 0;
			return;
		}
#endif

#ifdef DEBUG
		for(auto& section : m_sections)
			Message("Section %s base: 0x%p | size: %d\n", section.m_szName.c_str(), section.m_pBase, section.m_iSize);

		Message("Initialized module %s base: 0x%p | size: %d\n", m_pszModule, m_base, m_size);
#endif
	}

	~CModule()
	{
		if (m_hModule)
			dlclose(m_hModule);
	}

	CModule(const CModule &) = delete;
	CModule &operator=(const CModule &) = delete;

	// False when the module failed to load or its image info could not be read
	bool IsValid() const { return m_hModule && m_base; }

	void *FindSignature(const byte *pData, const byte *pMask, size_t iSigLength, int &error)
	{
		unsigned char *pMemory;
		void *return_addr = nullptr;
		error = 0;

		if (!IsValid() || !pData || !pMask || iSigLength == 0 || iSigLength > m_size)
		{
			error = SIG_NOT_FOUND;
			return nullptr;
		}

		pMemory = (byte*)m_base;

		for (size_t i = 0; i <= m_size - iSigLength; i++)
		{
			size_t Matches = 0;
			while (!pMask[Matches] || *(pMemory + i + Matches) == pData[Matches])
			{
				Matches++;
				if (Matches == iSigLength)
				{
					if (return_addr)
					{
						error = SIG_FOUND_MULTIPLE;
						return return_addr;
					}

					return_addr = (void *)(pMemory + i);
					break;
				}
			}
		}

		if (!return_addr)
			error = SIG_NOT_FOUND;

		return return_addr;
	}

	void *FindInterface(const char *name)
	{
		if (!IsValid())
		{
			Panic("Cannot find %s, module %s is not loaded\n", name, m_pszModule);
			return nullptr;
		}

		CreateInterfaceFn fn = (CreateInterfaceFn)dlsym(m_hModule, "CreateInterface");

		if (!fn)
		{
			Panic("Could not find CreateInterface in %s\n", m_pszModule);
			return nullptr;
		}

		void *pInterface = fn(name, nullptr);

		if (!pInterface)
		{
			Panic("Could not find %s in %s\n", name, m_pszModule);
			return nullptr;
		}

		Message("Found interface %s in %s\n", name, m_pszModule);

		return pInterface;
	}

	Section* GetSection(const std::string_view name)
	{
		for (auto& section : m_sections)
		{
			if (section.m_szName == name)
				return &section;
		}

		return nullptr;
	}
#ifdef _WIN32
	void InitializeSections();
#endif
public:
	const char *m_pszModule;
	const char* m_pszPath;
	HINSTANCE m_hModule = nullptr;
	void* m_base = nullptr;
	size_t m_size = 0;
	std::vector<Section> m_sections;
};