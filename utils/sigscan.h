/**
 * =============================================================================
 * GameBanFix
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
#include <cstddef>
#include <cstdint>

inline bool ScanForSignature(const uint8_t *pStart, size_t iSize, const uint8_t *pData, const uint8_t *pMask, size_t iSigLength, void *&pFound)
{
	if (!pStart || !pData || !pMask || iSigLength == 0 || iSize < iSigLength)
		return false;

	const uint8_t *pMatch = static_cast<const uint8_t *>(pFound);

	for (size_t i = 0; i <= iSize - iSigLength; i++)
	{
		size_t iMatched = 0;
		while (!pMask[iMatched] || pStart[i + iMatched] == pData[iMatched])
		{
			if (++iMatched == iSigLength)
				break;
		}

		if (iMatched != iSigLength)
			continue;

		if (pMatch)
			return true;

		pMatch = pStart + i;
		pFound = const_cast<uint8_t *>(pMatch);
	}

	return false;
}
