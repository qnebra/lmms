/*
 * VstPluginPool.h - singleton pool for sharing VST plugin instances
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

#ifndef _VST_PLUGIN_POOL_H
#define _VST_PLUGIN_POOL_H

#include <QMap>
#include <QMutex>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QString>

#include "vstbase_export.h"

namespace lmms
{

class VstPlugin;

/**
 * @brief Singleton pool for sharing VST plugin instances across multiple users
 * 
 * This pool allows multiple VstPlugin references to share a single RemoteVstPlugin
 * process when they use the same VST DLL path. This significantly reduces memory
 * usage and improves loading times when the same plugin is used multiple times.
 * 
 * Key features:
 * - Thread-safe with QMutex protection
 * - Uses QWeakPointer for automatic cleanup when all references are destroyed
 * - Per-DLL pooling based on absolute file paths
 * - Debug logging for troubleshooting
 */
class VSTBASE_EXPORT VstPluginPool
{
public:
	/**
	 * @brief Get the singleton instance of the pool
	 * @return Reference to the singleton instance
	 */
	static VstPluginPool& instance();

	/**
	 * @brief Get existing plugin instance or create a new one
	 * @param pluginPath Absolute path to the VST DLL/so file
	 * @return Shared pointer to VstPlugin instance
	 * 
	 * If a plugin with the same path already exists in the pool and is still alive,
	 * returns a shared reference to it. Otherwise, creates a new instance and adds
	 * it to the pool.
	 */
	QSharedPointer<VstPlugin> getOrCreate(const QString& pluginPath);

	/**
	 * @brief Remove expired weak pointers from the pool
	 * 
	 * Called automatically by getOrCreate(), but can be called manually
	 * for explicit cleanup.
	 */
	void cleanup();

	/**
	 * @brief Get the number of active pooled instances
	 * @return Count of plugins currently in the pool
	 */
	int activeCount() const;

private:
	VstPluginPool() = default;
	~VstPluginPool() = default;

	// Prevent copying
	VstPluginPool(const VstPluginPool&) = delete;
	VstPluginPool& operator=(const VstPluginPool&) = delete;

	QMap<QString, QWeakPointer<VstPlugin>> m_pool;
	mutable QMutex m_mutex;
};

} // namespace lmms

#endif // _VST_PLUGIN_POOL_H
