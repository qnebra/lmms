/*
 * InteractiveModelView.h - TODO
 *
 * Copyright (c) 2024 szeli1 <TODO/at/gmail/dot/com>
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

#include "InteractiveModelView.h"

#include <algorithm>

#include <QKeyEvent>
#include <QKeySequence> // displaying qt key names
#include <QPainter> // drawAutoHighlight()
#include <QPainterPath> // drawAutoHighlight()

#include "GuiApplication.h"
#include "MainWindow.h"
#include "SimpleTextFloat.h"

namespace lmms::gui
{

std::unique_ptr<QColor> InteractiveModelView::s_highlightColor = std::make_unique<QColor>();
QTimer* InteractiveModelView::s_highlightTimer = nullptr;

SimpleTextFloat* InteractiveModelView::s_simpleTextFloat = nullptr;
std::list<InteractiveModelView*> InteractiveModelView::s_interactiveWidgets;

InteractiveModelView::InteractiveModelView(QWidget* widget) :
	QWidget(widget),
	m_isHighlighted(false),
	m_lastShortcutCounter(0),
	m_focusedBeforeWidget(nullptr)
{
	s_interactiveWidgets.push_back(this);

	m_lastShortcut.reset();
}

InteractiveModelView::~InteractiveModelView()
{
	auto it = std::find(s_interactiveWidgets.begin(), s_interactiveWidgets.end(), this);
	if (it != s_interactiveWidgets.end())
	{
		s_interactiveWidgets.erase(it);
	}
}

void InteractiveModelView::startHighlighting(Clipboard::StringPairDataType dataType)
{
	if (s_highlightTimer == nullptr)
	{
		s_highlightTimer = new QTimer(getGUI()->mainWindow());
		s_highlightTimer->setSingleShot(true);
		QObject::connect(s_highlightTimer, &QTimer::timeout, timerStopHighlighting);
	}
	for (auto it = s_interactiveWidgets.begin(); it != s_interactiveWidgets.end(); ++it)
	{
		(*it)->overrideSetIsHighlighted((*it)->canAcceptClipboardData(dataType));
	}
	s_highlightTimer->start(20000);
}

void InteractiveModelView::stopHighlighting()
{
	for (auto it = s_interactiveWidgets.begin(); it != s_interactiveWidgets.end(); ++it)
	{
		(*it)->overrideSetIsHighlighted(false);
	}
}

void InteractiveModelView::showMessage(QString& message)
{
	if (s_simpleTextFloat == nullptr)
	{
		// we don't own this object, so we do not need to delete it
		s_simpleTextFloat = new SimpleTextFloat();
	}
	s_simpleTextFloat->setText(message);
	s_simpleTextFloat->moveToGlobal(QPoint(getGUI()->mainWindow()->pos().x() + 2,
		getGUI()->mainWindow()->pos().y() + getGUI()->mainWindow()->height()));
	s_simpleTextFloat->showWithDelay(0, 60000);
}

void InteractiveModelView::hideMessage()
{
	if (s_simpleTextFloat == nullptr)
	{
		s_simpleTextFloat = new SimpleTextFloat();
	}
	s_simpleTextFloat->hide();
}

QColor InteractiveModelView::getHighlightColor()
{
	return *s_highlightColor;
}

void InteractiveModelView::setHighlightColor(QColor& color)
{
	s_highlightColor = std::make_unique<QColor>(color);
}

void InteractiveModelView::keyPressEvent(QKeyEvent* event)
{
	std::vector<ModelShortcut> shortcuts(getShortcuts());

	size_t foundIndex = 0;
	unsigned int minMaxTimes = 0;
	bool found = false;

	// if the last shortcut's keys mach the current keys
	if (doesShortcutMatch(&m_lastShortcut, event))
	{
		// find the highest m_times or
		// the shortcut that's m_times == m_lastShortcutCounter
		for (size_t i = 0; i < shortcuts.size(); i++)
		{
			if (doesShortcutMatch(&shortcuts[i], event))
			{
				// selecting the shortcut with the largest m_times
				if (found == false || minMaxTimes < shortcuts[i].m_times)
				{
					foundIndex = i;
					minMaxTimes = shortcuts[i].m_times;
				}
				found = true;
			
				if (m_lastShortcutCounter == shortcuts[i].m_times)
				{
					m_lastShortcutCounter = shortcuts[i].m_shouldLoop ? 0 : m_lastShortcutCounter + 1;
					foundIndex = i;
					break;
				}
			}
		}
	}
	else
	{
		// find the lowest m_times
		for (size_t i = 0; i < shortcuts.size(); i++)
		{
			if (doesShortcutMatch(&shortcuts[i], event))
			{
				// selecting the shortcut with the largest m_times
				if (found == false || minMaxTimes > shortcuts[i].m_times)
				{
					foundIndex = i;
					minMaxTimes = shortcuts[i].m_times;
				}
				m_lastShortcut = shortcuts[i];
				m_lastShortcutCounter = 1;
				found = true;
			}
		}
	}
	if (found)
	{
		QString message = shortcuts[foundIndex].m_shortcutDescription;
		showMessage(message);
		processShortcutPressed(foundIndex, event);
	}
	else
	{
		// reset focus
		if (m_focusedBeforeWidget && event->key() != Qt::Key_Control
			&& event->key() != Qt::Key_Shift
			&& event->key() != Qt::Key_Alt
			&& event->key() != Qt::Key_AltGr)
		{
			m_focusedBeforeWidget->setFocus();
		}
	}
}

void InteractiveModelView::enterEvent(QEvent* event)
{
	m_lastShortcutCounter = 0;
	m_lastShortcut.reset();

	QString message = getShortcutMessage();
	showMessage(message);

	if (isVisible())
	{
		m_focusedBeforeWidget = QApplication::focusWidget();
		// focus on this widget so keyPressEvent works
		setFocus();
	}
}

void InteractiveModelView::leaveEvent(QEvent* event)
{
	hideMessage();
}

void InteractiveModelView::overrideSetIsHighlighted(bool isHighlighted)
{
	setIsHighlighted(isHighlighted);
}

void InteractiveModelView::drawAutoHighlight(QPainter* painter)
{
	if (getIsHighlighted())
	{
		QColor fillColor = *s_highlightColor;
		fillColor.setAlpha(70);
		painter->fillRect(QRect(1, 1, width() - 2, height() - 2), fillColor);

		painter->setPen(QPen(*s_highlightColor, 2));
		painter->drawLine(1, 1, 5, 1);
		painter->drawLine(1, 1, 1, 5);
		painter->drawLine(width() - 2, height() - 2, width() - 6, height() - 2);
		painter->drawLine(width() - 2, height() - 2, width() - 2, height() - 6);
	}
}

QString InteractiveModelView::buildShortcutMessage()
{
	QString message = "";
	std::vector<ModelShortcut> shortcuts(getShortcuts());
	for (size_t i = 0; i < shortcuts.size(); i++)
	{
		message = message + QString("\"") + QKeySequence(shortcuts[i].m_modifier).toString()
			+ QKeySequence(shortcuts[i].m_key).toString();
		if (shortcuts[i].m_times > 0)
		{
			message = message + QString(" (x") + QString::number(shortcuts[i].m_times + 1) + QString(")");
		}
		message = message + QString("\": ")
			+ shortcuts[i].m_shortcutDescription;
		if (i + 1 < shortcuts.size())
		{
			message = message + QString(", ");
		}
	}
	return message;
}

bool InteractiveModelView::getIsHighlighted() const
{
	return m_isHighlighted;
}

void InteractiveModelView::setIsHighlighted(bool isHighlighted)
{
	if (m_isHighlighted != isHighlighted)
	{
		m_isHighlighted = isHighlighted;
		if (isVisible())
		{
			update();
		}
	}
}

bool InteractiveModelView::doesShortcutMatch(const ModelShortcut* shortcut, QKeyEvent* event) const
{
	return shortcut->m_key == event->key() && (event->modifiers() & shortcut->m_modifier);
}

bool InteractiveModelView::doesShortcutMatch(const ModelShortcut* shortcutA, const ModelShortcut* shortcutB) const
{
	return shortcutA->m_key == shortcutB->m_key && (shortcutA->m_modifier & shortcutB->m_modifier);
}

} // namespace lmms::gui
