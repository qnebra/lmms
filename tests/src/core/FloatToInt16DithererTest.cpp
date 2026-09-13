/*
 * FloatToInt16DithererTest.cpp
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

#include <QObject>
#include <QtTest>

#include <limits>

#include "FloatToInt16Ditherer.h"


class FloatToInt16DithererTest : public QObject
{
	Q_OBJECT

private slots:
	void DisabledDither_RoundsAndClips()
	{
		using namespace lmms;

		const auto ditherer = FloatToInt16Ditherer{false};
		const auto lsb = 1.0f / static_cast<sample_t>(std::numeric_limits<int_sample_t>::max());

		QCOMPARE(ditherer.convert(0.5f * lsb), static_cast<int_sample_t>(1));
		QCOMPARE(ditherer.convert(-0.5f * lsb), static_cast<int_sample_t>(-1));
		QCOMPARE(ditherer.convert(2.0f), std::numeric_limits<int_sample_t>::max());
		QCOMPARE(ditherer.convert(-2.0f), static_cast<int_sample_t>(-std::numeric_limits<int_sample_t>::max()));
	}

	void EnabledDither_DecorrelatesHalfLsbSamples()
	{
		using namespace lmms;

		auto ditherer = FloatToInt16Ditherer{true};
		const auto lsb = 1.0f / static_cast<sample_t>(std::numeric_limits<int_sample_t>::max());

		bool sawZero = false;
		bool sawOne = false;

		for (int i = 0; i < 64; ++i)
		{
			const auto value = ditherer.convert(0.5f * lsb);
			sawZero |= value == 0;
			sawOne |= value == 1;
		}

		QVERIFY(sawZero);
		QVERIFY(sawOne);
	}
};

QTEST_GUILESS_MAIN(FloatToInt16DithererTest)
#include "FloatToInt16DithererTest.moc"
