/*
 * TrackLabelButton.cpp - implementation of class trackLabelButton, a label
 *                          which is renamable by double-clicking it
 *
 * Copyright (c) 2004-2008 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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


#include "TrackLabelButton.h"

#include <QMouseEvent>
#include <QHBoxLayout>
#include <QMenu>

#include "ConfigManager.h"
#include "embed.h"
#include "InstrumentTrackView.h"
#include "Instrument.h"
#include "InstrumentTrack.h"
#include "RenameDialog.h"
#include "TrackRenameLineEdit.h"
#include "TrackView.h"
#include "Track.h"

namespace lmms::gui
{

TrackLabelButton::TrackLabelButton( TrackView * _tv, QWidget * _parent ) :
	QToolButton( _parent ),
	m_trackView( _tv ),
	m_iconName()
{
	setAttribute( Qt::WA_OpaquePaintEvent, true );
	setAcceptDrops( true );
	setCursor( QCursor( embed::getIconPixmap( "hand" ), 3, 3 ) );
	setToolButtonStyle( Qt::ToolButtonTextBesideIcon );

	QWidget *container = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(30, 0, 5, 0);
    layout->setSpacing(0);
	
	m_renameLineEdit = new TrackRenameLineEdit(container);
	auto font = QFont();
	font.setPointSize(8);
	m_renameLineEdit->setFont(font);
	m_renameLineEdit->setStyleSheet("background: transparent;");
	m_renameLineEdit->setText(m_trackView->m_track->name());
	m_renameLineEdit->setEnabled(false);
	layout->addWidget(m_renameLineEdit);

	if (isInCompactMode())
	{
		setFixedSize( 32, 29 );
	}
	else
	{
		setFixedSize( 160, 29 );
		connect( m_renameLineEdit, SIGNAL(editingFinished()), this, SLOT(renameFinished()));
	}
	
	setIconSize( QSize( 24, 24 ) );
	container->setLayout(layout);
	container->setGeometry(0, 0, width(), height());
	container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	connect( m_trackView->getTrack(), SIGNAL(dataChanged()), this, SLOT(updateName()));
	connect( m_trackView->getTrack(), SIGNAL(nameChanged()), this, SLOT(updateName()));
}




void TrackLabelButton::rename()
{
	if (isInCompactMode())
	{
		QString txt = m_trackView->getTrack()->name();
		RenameDialog renameDlg( txt );
		renameDlg.exec();
		if(txt != m_trackView->getTrack()->name())
		{
			m_trackView->getTrack()->setName( txt );
		}
	}
	else
	{
		QString txt = m_trackView->getTrack()->name();
		m_renameLineEdit->setStyleSheet("");
		m_renameLineEdit->setEnabled(true);
		m_renameLineEdit->setText( txt );
		m_renameLineEdit->selectAll();
		m_renameLineEdit->setFocus();
	}
}




void TrackLabelButton::renameFinished()
{
	m_renameLineEdit->setEnabled(false);
	m_renameLineEdit->setStyleSheet("background: transparent;");

	if (m_renameLineEdit->text() == "")
	{
		m_renameLineEdit->setText(m_trackView->getTrack()->name());
		return;
	}

	if (!isInCompactMode())
	{
		m_renameLineEdit->clearFocus();
		if( m_renameLineEdit->text() != m_trackView->getTrack()->name() )
		{
			m_trackView->getTrack()->setName( m_renameLineEdit->text() );
		}
	}
}




void TrackLabelButton::updateName()
{
	m_renameLineEdit->setText(m_trackView->getTrack()->name());
}




void TrackLabelButton::dragEnterEvent( QDragEnterEvent * _dee )
{
	m_trackView->dragEnterEvent( _dee );
}




void TrackLabelButton::dropEvent( QDropEvent * _de )
{
	m_trackView->dropEvent( _de );
	setChecked( true );
}




void TrackLabelButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    QAction *renameAction = menu.addAction(tr("Rename"));
    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == renameAction) {
        rename();
    }
}




void TrackLabelButton::mousePressEvent( QMouseEvent * _me )
{
	m_buttonRect = QRect( this->mapToGlobal( pos() ), size() );
	_me->ignore();
}




void TrackLabelButton::mouseDoubleClickEvent( QMouseEvent * _me )
{
	rename();
}




void TrackLabelButton::mouseReleaseEvent( QMouseEvent *_me )
{
	if( m_buttonRect.contains( _me->globalPos(), true ) && !m_renameLineEdit->isEnabled() )
	{
		QToolButton::mousePressEvent( _me );
	}
	QToolButton::mouseReleaseEvent( _me );
	_me->ignore();
}


void TrackLabelButton::paintEvent(QPaintEvent* pe)
{
	if (m_trackView->getTrack()->type() == Track::Type::Instrument)
	{
		auto it = dynamic_cast<InstrumentTrack*>(m_trackView->getTrack());
		const PixmapLoader* pl;
		auto get_logo = [](InstrumentTrack* it) -> const PixmapLoader*
		{
			return it->instrument()->key().isValid()
				? it->instrument()->key().logo()
				: it->instrument()->descriptor()->logo;
		};
		if (it && it->instrument() &&
			it->instrument()->descriptor() &&
			(pl = get_logo(it)))
		{
			if (pl->pixmapName() != m_iconName)
			{
				m_iconName = pl->pixmapName();
				setIcon(pl->pixmap());
			}
		}
	}
	QToolButton::paintEvent(pe);
}




bool TrackLabelButton::isInCompactMode() const
{
	return ConfigManager::inst()->value("ui", "compacttrackbuttons").toInt();
}

} // namespace lmms::gui
