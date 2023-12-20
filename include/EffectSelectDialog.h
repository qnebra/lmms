/*
 * EffectSelectDialog.h - dialog to choose effect plugin
 *
 * Copyright (c) 2006-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * Copyright (c) 2023 Lost Robot <r94231/at/gmail.com>
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

#ifndef EFFECT_SELECT_DIALOG_H
#define EFFECT_SELECT_DIALOG_H

#include "Effect.h"

#include <QDialog>
#include <QHeaderView>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QRegExp>
#include <QScrollArea>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>

namespace lmms::gui
{

class DualColumnFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT
public:
	DualColumnFilterProxyModel(QObject* parent = nullptr) : QSortFilterProxyModel(parent)
	{
		m_effectTypeFilter = "";
	}

	void setEffectTypeFilter(const QString& filter)
	{
		m_effectTypeFilter = filter;
		invalidateFilter();
	}

protected:
	bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override
	{
		QModelIndex nameIndex = sourceModel()->index(source_row, 0, source_parent);
		QModelIndex typeIndex = sourceModel()->index(source_row, 1, source_parent);

		QString name = sourceModel()->data(nameIndex, Qt::DisplayRole).toString();
		QString type = sourceModel()->data(typeIndex, Qt::DisplayRole).toString();

		QRegExp nameRegExp(filterRegExp());
		nameRegExp.setCaseSensitivity(Qt::CaseInsensitive);

		bool nameFilterPassed = nameRegExp.indexIn(name) != -1;
		bool typeFilterPassed = (m_effectTypeFilter.isEmpty() || type.contains(m_effectTypeFilter, Qt::CaseInsensitive));

		return nameFilterPassed && typeFilterPassed;
	}

private:
	QString m_effectTypeFilter;
};


class EffectSelectDialog : public QDialog
{
	Q_OBJECT
public:
	EffectSelectDialog(QWidget* _parent);
	~EffectSelectDialog() override;

	Effect* instantiateSelectedPlugin(EffectChain* _parent);

protected slots:
	void acceptSelection();
	void rowChanged(const QModelIndex&, const QModelIndex&);
	void updateSelection();
	
	bool eventFilter(QObject* obj, QEvent* event) override;

private:
	EffectKeyList m_effectKeys;
	EffectKey m_currentSelection;

	QStandardItemModel m_sourceModel;
	DualColumnFilterProxyModel m_model;
	QWidget* m_descriptionWidget;
	QTableView* m_pluginList;
	QScrollArea* m_scrollArea;
	QLineEdit* m_filterEdit;
};

} // namespace lmms::gui

#endif

