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
#include "detours.h"

#include <memory>

#include "tier0/memdbgon.h"

#ifdef _WIN32
using CheckSteamBanHook = KHook::Function<void, void *, void *, void *, void *>;
#else
using CheckSteamBanHook = KHook::Function<void, void *, void *, void *, void *, void *, void *>;
#endif

struct CFunctionHook : CheckSteamBanHook
{
	CFunctionHook(CheckSteamBanHook::fnCallback pre, CheckSteamBanHook::fnCallback post) : CheckSteamBanHook(pre, post) {}

	bool IsHooked() const { return _associated_hook_id != KHook::INVALID_HOOK; }
};

static std::unique_ptr<CFunctionHook> g_pCheckSteamBanHook;

bool InitDetours(CGameConfig *gameConfig)
{
	const char *pszName = "GameSystem_Think_CheckSteamBan";

	void *pAddress = gameConfig->ResolveSignature(pszName);
	if (!pAddress)
		return false;

	g_pCheckSteamBanHook = std::make_unique<CFunctionHook>(nullptr, Post_GameSystem_Think_CheckSteamBan);
	g_pCheckSteamBanHook->Configure(pAddress);

	if (!g_pCheckSteamBanHook->IsHooked())
	{
		Panic("Failed to hook %s at 0x%p\n", pszName, pAddress);
		g_pCheckSteamBanHook.reset();
		return false;
	}

	Message("Detoured %s at 0x%p\n", pszName, pAddress);
	return true;
}

void FlushAllDetours()
{
	g_pCheckSteamBanHook.reset();
}

// Implementation shared by @aiolos1045
static void PurgeGcBanInformation()
{
	auto pMap = addresses::sm_mapGcBanInformation;
	if (!pMap)
		return;

	// After player has been kicked, remove any ban entries, to prevent spreading to all new joining players
	if (pMap->Count() > 0)
		pMap->RemoveAll();
}

#ifdef _WIN32
KHook::Return<void> Post_GameSystem_Think_CheckSteamBan(void *, void *, void *, void *)
#else
KHook::Return<void> Post_GameSystem_Think_CheckSteamBan(void *, void *, void *, void *, void *, void *)
#endif
{
	PurgeGcBanInformation();
	return { KHook::Action::Ignore };
}
