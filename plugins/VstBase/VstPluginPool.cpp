/*
 * VstPluginPool.cpp - singleton pool for sharing VST plugin instances
 *
 * Copyright (c) 2026 LMMS Developers
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

#include "VstPluginPool.h"
#include "VstPlugin.h"

#include <QDebug>
#include <QMutexLocker>

namespace lmms
{

VstPluginPool& VstPluginPool::instance()
{
	// Thread-safe singleton using static local variable (C++11)
	static VstPluginPool instance;
	return instance;
}

QSharedPointer<VstPlugin> VstPluginPool::getOrCreate(const QString& pluginPath)
{
	QMutexLocker locker(&m_mutex);

	// Clean up expired entries periodically
	cleanup();

	// Check if plugin already exists in pool
	auto it = m_pool.find(pluginPath);
	if (it != m_pool.end())
	{
		// Try to lock the weak pointer
		QSharedPointer<VstPlugin> shared = it.value().toStrongRef();
		if (shared)
		{
			qDebug() << "VstPluginPool: Reusing existing plugin instance for" << pluginPath;
			return shared;
		}
		else
		{
			// Weak pointer expired, remove from pool
			m_pool.erase(it);
		}
	}

	// Create new plugin instance
	qDebug() << "VstPluginPool: Creating new plugin instance for" << pluginPath;
	QSharedPointer<VstPlugin> newPlugin(new VstPlugin(pluginPath));

	// Add to pool as weak pointer
	m_pool[pluginPath] = newPlugin.toWeakRef();

	return newPlugin;
}

void VstPluginPool::cleanup()
{
	// Note: Caller should already hold mutex lock
	auto it = m_pool.begin();
	while (it != m_pool.end())
	{
		if (it.value().isNull())
		{
			qDebug() << "VstPluginPool: Removing expired entry for" << it.key();
			it = m_pool.erase(it);
		}
		else
		{
			++it;
		}
	}
}

int VstPluginPool::activeCount() const
{
	QMutexLocker locker(&m_mutex);
	int count = 0;
	for (auto it = m_pool.constBegin(); it != m_pool.constEnd(); ++it)
	{
		if (!it.value().isNull())
		{
			++count;
		}
	}
	return count;
}

} // namespace lmms
