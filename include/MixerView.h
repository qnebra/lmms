/*
 * MixerView.h - effect-mixer-view for LMMS
 *
 * Copyright (c) 2008-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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

#ifndef MIXER_VIEW_H
#define MIXER_VIEW_H

#include <QWidget>
#include <QHBoxLayout>
#include <QStackedLayout>
#include <QScrollArea>

#include "ModelView.h"
#include "Engine.h"
#include "Fader.h"
#include "PixmapButton.h"
#include "embed.h"
#include "EffectRackView.h"

namespace lmms::gui
{
class LMMS_EXPORT MixerView : public QWidget, public ModelView,
					public SerializingObjectHook
{
	Q_OBJECT
public:
	MixerView();
	~MixerView() override;

	void keyPressEvent(QKeyEvent * e) override;

	void saveSettings( QDomDocument & _doc, QDomElement & _this ) override;
	void loadSettings( const QDomElement & _this ) override;

	inline MixerChannelView* currentMixerChannel()
	{
		return m_currentMixerChannel;
	}

	inline MixerChannelView * channelView(int index)
	{
		return m_mixerChannelViews[index];
	}


	void setCurrentMixerChannel(MixerChannelView* channel);
	void setCurrentMixerChannel(int channel);

	void clear();


	// display the send button and knob correctly
	void updateMixerChannel(int index);

	// notify the view that a mixer channel was deleted
	void deleteChannel(int index);

	// delete all unused channels
	void deleteUnusedChannels();

	// move the channel to the left or right
	void moveChannelLeft(int index);
	void moveChannelLeft(int index, int focusIndex);
	void moveChannelRight(int index);

	void renameChannel(int index);

	// make sure the display syncs up with the mixer.
	// useful for loading projects
	void refreshDisplay();

public slots:
	int addNewChannel();

protected:
	void closeEvent( QCloseEvent * _ce ) override;
	
private slots:
	void updateFaders();
	void toggledSolo();

private:
	QVector<MixerChannelView *> m_mixerChannelViews;

	MixerChannelView* m_currentMixerChannel;

	QScrollArea * channelArea;
	QHBoxLayout * chLayout;
	QWidget * m_channelAreaWidget;
	QStackedLayout * m_racksLayout;
	QWidget * m_racksWidget;

	void updateMaxChannelSelector();
	
	friend class MixerChannelView;
} ;


} // namespace lmms::gui

#endif
