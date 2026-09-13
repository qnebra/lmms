/*
 * FloatToInt16Ditherer.h - dithered float-to-int16 sample conversion
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

#ifndef LMMS_FLOAT_TO_INT16_DITHERER_H
#define LMMS_FLOAT_TO_INT16_DITHERER_H

#include <algorithm>
#include <cmath>
#include <limits>

#include "LmmsTypes.h"


namespace lmms
{

class FloatToInt16Ditherer
{
public:
	explicit FloatToInt16Ditherer(bool enabled = isEnabledByDefault()) noexcept :
		m_enabled(enabled)
	{
	}

	int_sample_t convert(sample_t sample) const noexcept
	{
		auto scaledSample = std::clamp(sample, -1.0f, 1.0f) * outputSampleMultiplier();
		if (m_enabled)
		{
			scaledSample += triangularDither();
		}

		auto quantizedSample = std::lround(scaledSample);
		quantizedSample = std::clamp(quantizedSample,
			static_cast<long>(std::numeric_limits<int_sample_t>::min()),
			static_cast<long>(std::numeric_limits<int_sample_t>::max()));

		return static_cast<int_sample_t>(quantizedSample);
	}

	static bool isEnabledByDefault();

private:
	static constexpr sample_t outputSampleMultiplier() noexcept
	{
		return static_cast<sample_t>(std::numeric_limits<int_sample_t>::max());
	}

	static sample_t triangularDither() noexcept;

	bool m_enabled;
};

} // namespace lmms

#endif // LMMS_FLOAT_TO_INT16_DITHERER_H
