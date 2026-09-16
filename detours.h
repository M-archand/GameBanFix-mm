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

#pragma once
#include "cdetour.h"


bool InitDetours(CGameConfig *gameConfig);
void FlushAllDetours();

#ifdef _WIN32
// Win64 passes the first four integer arguments in rcx, rdx, r8, r9.
void FASTCALL Detour_GameSystem_Think_CheckSteamBan(void *a1, void *a2, void *a3, void *a4);
#else
// SysV passes the first six in rdi, rsi, rdx, rcx, r8, r9.
void FASTCALL Detour_GameSystem_Think_CheckSteamBan(void *a1, void *a2, void *a3, void *a4, void *a5, void *a6);
#endif