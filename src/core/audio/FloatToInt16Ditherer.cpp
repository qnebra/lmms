/*
 * FloatToInt16Ditherer.cpp - dithered float-to-int16 sample conversion
 *
 * This file is part of LMMS - https://lmms.io
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */

#include "FloatToInt16Ditherer.h"

#include <cstdint>

#include "ConfigManager.h"


namespace lmms
{

namespace
{

constexpr char FloatToInt16DitheringSection[] = "audioengine";
constexpr char FloatToInt16DitheringKey[] = "floattoint16dither";
constexpr std::uint32_t FloatToInt16DitherSeed = 0x6d6d7301u;
constexpr sample_t U24ToUnitScale = 1.0f / 16777216.0f;

sample_t nextRandomSample() noexcept
{
	thread_local std::uint32_t s_state = FloatToInt16DitherSeed;

	s_state = s_state * 1664525u + 1013904223u;
	return static_cast<sample_t>((s_state >> 8) * U24ToUnitScale);
}

} // namespace


bool FloatToInt16Ditherer::isEnabledByDefault()
{
	return ConfigManager::inst()->value(FloatToInt16DitheringSection, FloatToInt16DitheringKey, "1").toInt() != 0;
}


sample_t FloatToInt16Ditherer::triangularDither() noexcept
{
	return 0.5f * (nextRandomSample() - nextRandomSample());
}

} // namespace lmms
