/*
 * BufferManager.cpp - A buffer caching/memory management system
 *
 * Copyright (c) 2017 Lukas W <lukaswhl/at/gmail.com>
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#include "BufferManager.h"

#include "LocklessAllocator.h"
#include "PlayHandle.h"
#include "SampleFrame.h"


namespace lmms
{

// Must be at least as large as PlayHandle::MaxNumber so that every
// concurrent play handle can have a pooled buffer without falling back to heap.
static constexpr std::size_t BufferPoolCapacity = PlayHandle::MaxNumber;

f_cnt_t BufferManager::s_framesPerPeriod;
LocklessAllocator* BufferManager::s_pool = nullptr;

void BufferManager::init( f_cnt_t fpp )
{
	s_framesPerPeriod = fpp;
	delete s_pool;
	s_pool = new LocklessAllocator(BufferPoolCapacity, fpp * sizeof(SampleFrame));
}


SampleFrame* BufferManager::acquire()
{
	// Try the pool first. available() is a relaxed load so it avoids a mutex
	// on the hot audio path. In the rare case of a race (another thread grabs
	// the last slot between our check and alloc()), alloc() returns nullptr
	// and we fall through to the heap fallback.
	if (s_pool && s_pool->available() > 0)
	{
		if (void* buf = s_pool->alloc())
		{
			return static_cast<SampleFrame*>(buf);
		}
	}
	return new SampleFrame[s_framesPerPeriod];
}



void BufferManager::release( SampleFrame* buf )
{
	if (s_pool && s_pool->isFromPool(buf))
	{
		s_pool->free(buf);
	}
	else
	{
		delete[] buf;
	}
}

} // namespace lmms
