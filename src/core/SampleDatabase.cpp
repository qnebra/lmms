/*
 * SampleDatabase.cpp
 *
 * Copyright (c) 2024 saker
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

#include "SampleDatabase.h"

#include <filesystem>

#include "FileSystemHelpers.h"
#include "SampleBuffer.h"

namespace lmms {
auto SampleDatabase::fetch(const std::filesystem::path& path) -> std::shared_ptr<SampleBuffer>
{
	const auto entry = AudioFileEntry{path, std::filesystem::last_write_time(path)};
	const auto it = s_audioFileMap.find(entry);

	if (it == s_audioFileMap.end())
	{
		const auto buffer = std::make_shared<SampleBuffer>(FileSystemHelpers::qStringFromPath(path));
		s_audioFileMap.insert(std::make_pair(entry, buffer));
		return buffer;
	}

	return it->second.lock();
}

auto SampleDatabase::fetch(const std::string& base64, int sampleRate) -> std::shared_ptr<SampleBuffer>
{
	const auto entry = Base64Entry{base64, sampleRate};
	const auto it = s_base64Map.find(entry);

	if (it == s_base64Map.end())
	{
		const auto buffer = std::make_shared<SampleBuffer>(QString::fromStdString(base64), sampleRate);
		s_base64Map.insert(std::make_pair(entry, buffer));
		return buffer;
	}

	return it->second.lock();
}
} // namespace lmms
