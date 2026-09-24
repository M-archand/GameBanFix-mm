/**
 * =============================================================================
 * CS2Fixes
 * Copyright (C) 2023-2024 Source2ZE
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

#include "addresses.h"
#include "utils/module.h"

#include <cstring>

#include "tier0/memdbgon.h"

extern CGameConfig *g_GameConfig;

namespace
{
	struct ModuleSlot
	{
		const char *m_pszLibrary;
		const char *m_pszPath;
		const char *m_pszModule;
		CModule *m_pModule = nullptr;
	};

	ModuleSlot s_modules[] = {
		{"engine", ROOTBIN, "engine2"},
		{"tier0", ROOTBIN, "tier0"},
		{"server", GAMEBIN, "server"},
		{"schemasystem", ROOTBIN, "schemasystem"},
		{"vscript", ROOTBIN, "vscript"},
		{"networksystem", ROOTBIN, "networksystem"},
		{"client", GAMEBIN, "client"},
#ifdef _WIN32
		{"hammer", ROOTBIN, "tools/hammer"},
#endif
	};

	bool IsModulePresent(const char *library)
	{
		if (strcmp(library, "client") == 0)
			return !CommandLine()->HasParm("-dedicated");
#ifdef _WIN32
		if (strcmp(library, "hammer") == 0)
			return CommandLine()->HasParm("-tools");
#endif
		return true;
	}

	bool SectionContains(const Section *pSection, uintptr_t start, size_t iSize)
	{
		if (!pSection)
			return false;

		uintptr_t base = (uintptr_t)pSection->m_pBase;
		return start >= base && iSize <= pSection->m_iSize && start - base <= pSection->m_iSize - iSize;
	}
}

CModule *modules::Get(const char *library)
{
	if (!library)
		return nullptr;

	for (auto &slot : s_modules)
	{
		if (strcmp(slot.m_pszLibrary, library) != 0)
			continue;

		if (!IsModulePresent(library))
			return nullptr;

		if (!slot.m_pModule)
			slot.m_pModule = new CModule(slot.m_pszPath, slot.m_pszModule);

		return slot.m_pModule;
	}

	return nullptr;
}

#define RESOLVE_SIG(gameConfig, name, variable) \
	variable = (decltype(variable))gameConfig->ResolveSignature(name);	\
	if (!variable)														\
	{																	\
		Panic("Failed to resolve %s\n", name);						\
		return false;													\
	}																	\
	Message("Found %s at 0x%p\n", name, variable);

bool addresses::Initialize(CGameConfig *g_GameConfig)
{
	return InitializeBanMap(g_GameConfig);
}

bool addresses::InitializeBanMap(CGameConfig* g_GameConfig)
{
	const char *pszName = "CCSGameRules__sm_mapGcBanInformation";
	addresses::sm_mapGcBanInformation = nullptr;

	// This signature directly points to the instruction referencing sm_mapGcBanInformation
	const uint8_t *pInsn = (const uint8_t *)g_GameConfig->ResolveSignature(pszName);

	if (!pInsn)
		return false;

	CModule *module = g_GameConfig->GetModule(pszName);

	if (!module)
		return false;

	const size_t iInsnLength = 7;

	bool bInCode = false;
	for (const auto &section : module->m_sections)
	{
		if (section.m_bExecutable && SectionContains(&section, (uintptr_t)pInsn, iInsnLength))
		{
			bInCode = true;
			break;
		}
	}

	if (!bInCode)
	{
		Panic("%s at 0x%p is not inside an executable section, refusing to load\n", pszName, pInsn);
		return false;
	}

	if ((pInsn[0] & 0xF8) != 0x48 || pInsn[1] != 0x8D || (pInsn[2] & 0xC7) != 0x05)
	{
		Panic("%s at 0x%p is not a RIP-relative lea (%02X %02X %02X), refusing to load\n", pszName, pInsn, pInsn[0], pInsn[1], pInsn[2]);
		return false;
	}

	int32_t disp;
	memcpy(&disp, pInsn + 3, sizeof(disp));

	uintptr_t target = (uintptr_t)(pInsn + iInsnLength) + (intptr_t)disp;

	const size_t iMapSize = sizeof(*addresses::sm_mapGcBanInformation);
	const Section *pSection = module->GetSection(".data");

	if (!pSection || pSection->m_bExecutable || !SectionContains(pSection, target, iMapSize))
		pSection = module->GetSection(".bss");

	if (!pSection || pSection->m_bExecutable || !SectionContains(pSection, target, iMapSize))
	{
		Panic("%s resolves to 0x%p, outside the module's .data and .bss sections, refusing to load\n", pszName, (void *)target);
		return false;
	}

	addresses::sm_mapGcBanInformation = (decltype(addresses::sm_mapGcBanInformation))target;

	Message("Found %s at 0x%p in %s\n", pszName, addresses::sm_mapGcBanInformation, pSection->m_szName.c_str());
	return true;
}

void addresses::Shutdown()
{
	for (auto &slot : s_modules)
	{
		delete slot.m_pModule;
		slot.m_pModule = nullptr;
	}

	addresses::sm_mapGcBanInformation = nullptr;
}