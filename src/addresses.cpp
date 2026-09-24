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
	addresses::sm_mapGcBanInformation = nullptr;

	// This signature directly points to the instruction referencing sm_mapGcBanInformation
	uintptr_t pAddr = (uintptr_t)g_GameConfig->ResolveSignature("CCSGameRules__sm_mapGcBanInformation");

	if (!pAddr)
		return false;

	// the opcode is 3 bytes so we skip those
	pAddr += 3;

	// Grab the offset as 4 bytes
	uint32 offset = *(uint32*)pAddr;

	// Go to the next instruction, which is what the relative address is based off
	pAddr += 4;

	// Get the real address
	addresses::sm_mapGcBanInformation = (decltype(addresses::sm_mapGcBanInformation))(pAddr + offset);

	if (!addresses::sm_mapGcBanInformation)
		return false;

	Message("Found %s at 0x%p\n", "CCSGameRules__sm_mapGcBanInformation", addresses::sm_mapGcBanInformation);
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