/*
 * SampleDatabase.h
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

#ifndef LMMS_SAMPLE_DATABASE_H
#define LMMS_SAMPLE_DATABASE_H

#include <QString>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "SampleBuffer.h"

namespace lmms {
class SampleDatabase
{
public:
	/**
		Fetches a sample from the database through a path to an audio file,
		and returns the stored buffer.

		If `path` exists in the database, its creation time and last write time
		is checked with what is currently in the database. If there is a mismatch, the sample is reloaded from disk, its
		entry in the database is updated, and the sample is returned.

		If `path` does not exist in the database, the sample is loaded from disk and
		then returned.

		If `path` does not exist on disk, an empty buffer is returned.
	 */
	auto fetch(const QString& path) -> std::shared_ptr<SampleBuffer>;

	/**
		Fetches a sample from the database through a Base64 string and a sample rate
		and returns the stored buffer.

		If an entry for a `base64` string with a certain `sampleRate` exists in the database, the stored sample is
		returned. Otherwise, if it does not exist in the database, the sample is loaded and then returned.
	 */
	auto fetch(const std::string& base64, int sampleRate) -> std::shared_ptr<SampleBuffer>;

private:
	struct AudioFileEntry
	{
		std::filesystem::path path;
		std::filesystem::file_time_type creationTime;
		std::filesystem::file_time_type lastWriteTime;
	};

	struct Base64Entry
	{
		std::string base64;
		int sampleRate;
	};

	std::unordered_map<AudioFileEntry, std::weak_ptr<SampleBuffer>> m_audioFileMap;
	std::unordered_map<Base64Entry, std::weak_ptr<SampleBuffer>> m_base64Map;
};
} // namespace lmms

#endif // LMMS_SAMPLE_DATABASE_H
