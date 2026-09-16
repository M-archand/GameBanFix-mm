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

#include "module.h"
#include "plat.h"

#include "tier0/memdbgon.h"

void CModule::InitializeSections()
{
	IMAGE_DOS_HEADER* pDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(m_hModule);
	IMAGE_NT_HEADERS* pNtHeader = reinterpret_cast<IMAGE_NT_HEADERS64*>(reinterpret_cast<uintptr_t>(m_hModule) + pDosHeader->e_lfanew);

	IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

	for (int i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		Section section;
		section.m_szName = (char*)pSectionHeader[i].Name;
		section.m_pBase = (void*)((uint8_t*)m_base + pSectionHeader[i].VirtualAddress);
		section.m_iSize = pSectionHeader[i].SizeOfRawData;

		m_sections.push_back(std::move(section));
	}
}