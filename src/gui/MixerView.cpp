/*
 * MixerView.cpp - effect-mixer-view for LMMS
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


#include <QLayout>
#include <QPushButton>
#include <QScrollArea>
#include <QStyle>
#include <QKeyEvent>

#include "lmms_math.h"

#include "MixerChannelView.h"
#include "MixerView.h"
#include "Knob.h"
#include "Mixer.h"
#include "GuiApplication.h"
#include "MainWindow.h"
#include "AudioEngine.h"
#include "InstrumentTrack.h"
#include "PatternStore.h"
#include "SampleTrack.h"
#include "SendButtonIndicator.h"
#include "Song.h"
#include "SubWindow.h"
#include "TrackContainer.h" // For TrackContainer::TrackList typedef

namespace lmms::gui
{


MixerView::MixerView() :
	QWidget(),
	ModelView( nullptr, this ),
	SerializingObjectHook()
{
	Mixer * m = Engine::mixer();
	m->setHook( this );

	//QPalette pal = palette();
	//pal.setColor( QPalette::Background, QColor( 72, 76, 88 ) );
	//setPalette( pal );

	setAutoFillBackground( true );
	setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );

	setWindowTitle( tr( "Mixer" ) );
	setWindowIcon( embed::getIconPixmap( "mixer" ) );

	// main-layout
	auto ml = new QHBoxLayout;

	// Set margins
	ml->setContentsMargins( 0, 4, 0, 0 );

	// Channel area
	m_channelAreaWidget = new QWidget;
	chLayout = new QHBoxLayout( m_channelAreaWidget );
	chLayout->setSizeConstraint( QLayout::SetMinimumSize );
	chLayout->setSpacing( 0 );
	chLayout->setMargin( 0 );
	m_channelAreaWidget->setLayout(chLayout);

	// create rack layout before creating the first channel
	m_racksWidget = new QWidget;
	m_racksLayout = new QStackedLayout( m_racksWidget );
	m_racksLayout->setContentsMargins( 0, 0, 0, 0 );
	m_racksWidget->setLayout( m_racksLayout );

	// add master channel
	m_mixerChannelViews.resize( m->numChannels() );
	m_mixerChannelViews[0] = new MixerChannelView(0, this, this);

	m_racksLayout->addWidget(m_mixerChannelViews[0]->m_effectRackView);

	MixerChannelView * masterView = m_mixerChannelViews[0];
	ml->addWidget(masterView, 0, Qt::AlignTop);

	QSize mixerChannelSize = masterView->size();

	// add mixer channels
	for( int i = 1; i < m_mixerChannelViews.size(); ++i )
	{
		m_mixerChannelViews[i] = new MixerChannelView(i, this, m_channelAreaWidget);
		chLayout->addWidget(m_mixerChannelViews[i]);
	}

	// add the scrolling section to the main layout
	// class solely for scroll area to pass key presses down
	class ChannelArea : public QScrollArea
	{
		public:
			ChannelArea( QWidget * parent, MixerView * mv ) :
				QScrollArea( parent ), m_mv( mv ) {}
			~ChannelArea() override = default;
			void keyPressEvent( QKeyEvent * e ) override
			{
				m_mv->keyPressEvent( e );
			}
		private:
			MixerView * m_mv;
	};
	channelArea = new ChannelArea( this, this );
	channelArea->setWidget( m_channelAreaWidget );
	channelArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
	channelArea->setFrameStyle( QFrame::NoFrame );
	channelArea->setMinimumWidth(mixerChannelSize.width() * 6);
	channelArea->setFixedHeight(mixerChannelSize.height() +
			style()->pixelMetric( QStyle::PM_ScrollBarExtent ) );
	ml->addWidget( channelArea, 1, Qt::AlignTop );

	// show the add new mixer channel button
	auto newChannelBtn = new QPushButton(embed::getIconPixmap("new_channel"), QString(), this);
	newChannelBtn->setObjectName( "newChannelBtn" );
	newChannelBtn->setFixedSize( mixerChannelSize );
	connect( newChannelBtn, SIGNAL(clicked()), this, SLOT(addNewChannel()));
	ml->addWidget( newChannelBtn, 0, Qt::AlignTop );


	// add the stacked layout for the effect racks of mixer channels
	ml->addWidget( m_racksWidget, 0, Qt::AlignTop | Qt::AlignRight );

	setCurrentMixerChannel(m_mixerChannelViews[0]);

	setLayout( ml );
	updateGeometry();

	// timer for updating faders
	connect( getGUI()->mainWindow(), SIGNAL(periodicUpdate()),
					this, SLOT(updateFaders()));


	// add ourself to workspace
	QMdiSubWindow * subWin = getGUI()->mainWindow()->addWindowedWidget( this );
	Qt::WindowFlags flags = subWin->windowFlags();
	flags &= ~Qt::WindowMaximizeButtonHint;
	subWin->setWindowFlags( flags );
	layout()->setSizeConstraint( QLayout::SetMinimumSize );
	subWin->layout()->setSizeConstraint( QLayout::SetMinAndMaxSize );

	parentWidget()->setAttribute( Qt::WA_DeleteOnClose, false );
	parentWidget()->move( 5, 310 );

	// we want to receive dataChanged-signals in order to update
	setModel( m );
}

MixerView::~MixerView()
{
	for (auto mixerChannelView : m_mixerChannelViews)
	{
		delete mixerChannelView;
	}
}



int MixerView::addNewChannel()
{
	// add new mixer channel and redraw the form.
	Mixer * mix = Engine::mixer();

	int newChannelIndex = mix->createChannel();
	m_mixerChannelViews.push_back(new MixerChannelView(newChannelIndex, this, m_channelAreaWidget));
	chLayout->addWidget(m_mixerChannelViews[newChannelIndex]);
	m_racksLayout->addWidget(m_mixerChannelViews[newChannelIndex]);

	updateMixerChannel(newChannelIndex);

	updateMaxChannelSelector();

	return newChannelIndex;
}


void MixerView::refreshDisplay()
{
	// delete all views and re-add them
	for( int i = 1; i<m_mixerChannelViews.size(); ++i )
	{
		chLayout->removeWidget(m_mixerChannelViews[i]);
		m_racksLayout->removeWidget(m_mixerChannelViews[i]->m_effectRackView);
	}
	m_channelAreaWidget->adjustSize();

	// re-add the views
	m_mixerChannelViews.resize(Engine::mixer()->numChannels());
	for( int i = 1; i < m_mixerChannelViews.size(); ++i )
	{
		m_mixerChannelViews[i] = new MixerChannelView(i, this, m_channelAreaWidget);
		chLayout->addWidget(m_mixerChannelViews[i]);
		m_racksLayout->addWidget( m_mixerChannelViews[i]->m_effectRackView);
	}

	// set selected mixer channel to 0
	setCurrentMixerChannel( 0 );

	// update all mixer lines
	for( int i = 0; i < m_mixerChannelViews.size(); ++i )
	{
		updateMixerChannel(i);
	}

	updateMaxChannelSelector();
}


// update the and max. channel number for every instrument
void MixerView::updateMaxChannelSelector()
{
	TrackContainer::TrackList songTracks = Engine::getSong()->tracks();
	TrackContainer::TrackList patternStoreTracks = Engine::patternStore()->tracks();

	for (const auto& trackList : {songTracks, patternStoreTracks})
	{
		for (const auto& track : trackList)
		{
			if (track->type() == Track::InstrumentTrack)
			{
				auto inst = (InstrumentTrack*)track;
				inst->mixerChannelModel()->setRange(0,
					m_mixerChannelViews.size()-1,1);
			}
			else if (track->type() == Track::SampleTrack)
			{
				auto strk = (SampleTrack*)track;
				strk->mixerChannelModel()->setRange(0,
					m_mixerChannelViews.size()-1,1);
			}
		}
	}
}


void MixerView::saveSettings( QDomDocument & _doc, QDomElement & _this )
{
	MainWindow::saveWidgetState( this, _this );
}




void MixerView::loadSettings( const QDomElement & _this )
{
	MainWindow::restoreWidgetState( this, _this );
}





void MixerView::toggledSolo()
{
	Engine::mixer()->toggledSolo();
}



void MixerView::setCurrentMixerChannel(MixerChannelView* channel)
{
	// select
	m_currentMixerChannel = channel;
	m_racksLayout->setCurrentWidget(m_mixerChannelViews[channel->channelIndex()]->m_effectRackView);

	// set up send knob
	for(int i = 0; i < m_mixerChannelViews.size(); ++i)
	{
		updateMixerChannel(i);
	}
}


void MixerView::updateMixerChannel(int index)
{
	Mixer * mix = Engine::mixer();

	// does current channel send to this channel?
	int selIndex = m_currentMixerChannel->channelIndex();
	auto thisLine = m_mixerChannelViews[index];
	thisLine->setToolTip( Engine::mixer()->mixerChannel( index )->m_name );

	FloatModel * sendModel = mix->channelSendModel(selIndex, index);
	if( sendModel == nullptr )
	{
		// does not send, hide send knob
		thisLine->m_sendKnob->setVisible(false);
	}
	else
	{
		// it does send, show knob and connect
		thisLine->m_sendKnob->setVisible(true);
		thisLine->m_sendKnob->setModel(sendModel);
	}

	// disable the send button if it would cause an infinite loop
	thisLine->m_sendButton->setVisible(! mix->isInfiniteLoop(selIndex, index));
	thisLine->m_sendButton->updateLightStatus();
	thisLine->update();
}


void MixerView::deleteChannel(int index)
{
	// can't delete master
	if( index == 0 ) return;

	// remember selected line
	int selLine = m_currentMixerChannel->channelIndex();

	// in case the deleted channel is soloed or the remaining
	// channels will be left in a muted state
	Engine::mixer()->clearChannel(index);

	// delete the real channel
	Engine::mixer()->deleteChannel(index);

	// delete the view
	chLayout->removeWidget(m_mixerChannelViews[index]);
	m_racksLayout->removeWidget( m_mixerChannelViews[index]);
	delete m_mixerChannelViews[index]->m_fader;
	delete m_mixerChannelViews[index]->m_muteButton;
	delete m_mixerChannelViews[index]->m_soloButton;
	// delete mixerLine later to prevent a crash when deleting from context menu
	m_mixerChannelViews[index]->hide();
	m_mixerChannelViews[index]->deleteLater();
	delete m_mixerChannelViews[index]->m_effectRackView;
	delete m_mixerChannelViews[index];
	m_channelAreaWidget->adjustSize();

	// make sure every channel knows what index it is
	for(int i=index + 1; i<m_mixerChannelViews.size(); ++i)
	{
		m_mixerChannelViews[i]->setChannelIndex(i - 1);
	}
	m_mixerChannelViews.remove(index);

	// select the next channel
	if( selLine >= m_mixerChannelViews.size() )
	{
		selLine = m_mixerChannelViews.size()-1;
	}
	setCurrentMixerChannel(selLine);

	updateMaxChannelSelector();
}



void MixerView::deleteUnusedChannels()
{
	TrackContainer::TrackList tracks;
	tracks += Engine::getSong()->tracks();
	tracks += Engine::patternStore()->tracks();

	std::vector<bool> inUse(m_mixerChannelViews.size(), false);

	//Populate inUse by checking the destination channel for every track
	for (Track* t: tracks)
	{
		//The channel that this track sends to. Since master channel is always in use,
		//setting this to 0 is a safe default (for tracks that don't sent to the mixer).
		int channel = 0;
		if (t->type() == Track::InstrumentTrack)
		{
			auto inst = dynamic_cast<InstrumentTrack*>(t);
			channel = inst->mixerChannelModel()->value();
		}
		else if (t->type() == Track::SampleTrack)
		{
			auto strack = dynamic_cast<SampleTrack*>(t);
			channel = strack->mixerChannelModel()->value();
		}
		inUse[channel] = true;
	}

	//Check all channels except master, delete those with no incoming sends
	for(int i = m_mixerChannelViews.size()-1; i > 0; --i)
	{
		if (!inUse[i] && Engine::mixer()->mixerChannel(i)->m_receives.isEmpty())
		{ deleteChannel(i); }
	}
}



void MixerView::moveChannelLeft(int index, int focusIndex)
{
	// can't move master or first channel left or last channel right
	if( index <= 1 || index >= m_mixerChannelViews.size() ) return;

	Mixer *m = Engine::mixer();

	// Move instruments channels
	m->moveChannelLeft( index );

	// Update widgets models
	m_mixerChannelViews[index]->setChannelIndex( index );
	m_mixerChannelViews[index - 1]->setChannelIndex( index - 1 );

	// Focus on new position
	setCurrentMixerChannel(focusIndex);
}



void MixerView::moveChannelLeft(int index)
{
	moveChannelLeft( index, index - 1 );
}



void MixerView::moveChannelRight(int index)
{
	moveChannelLeft( index + 1, index + 1 );
}


void MixerView::renameChannel(int index)
{
	m_mixerChannelViews[index]->renameChannel();
}



void MixerView::keyPressEvent(QKeyEvent * e)
{
	switch(e->key())
	{
		case Qt::Key_Delete:
			deleteChannel(m_currentMixerChannel->channelIndex());
			break;
		case Qt::Key_Left:
			if( e->modifiers() & Qt::AltModifier )
			{
				moveChannelLeft(m_currentMixerChannel->channelIndex());
			}
			else
			{
				// select channel to the left
				setCurrentMixerChannel(m_currentMixerChannel->channelIndex() - 1);
			}
			break;
		case Qt::Key_Right:
			if( e->modifiers() & Qt::AltModifier )
			{
				moveChannelRight(m_currentMixerChannel->channelIndex());
			}
			else
			{
				// select channel to the right
				setCurrentMixerChannel(m_currentMixerChannel->channelIndex() + 1);
			}
			break;
		case Qt::Key_Insert:
			if ( e->modifiers() & Qt::ShiftModifier )
			{
				addNewChannel();
			}
			break;
		case Qt::Key_Enter:
		case Qt::Key_Return:
		case Qt::Key_F2:
			renameChannel(m_currentMixerChannel->channelIndex());
			break;
	}
}



void MixerView::closeEvent( QCloseEvent * _ce )
 {
	if( parentWidget() )
	{
		parentWidget()->hide();
	}
	else
	{
		hide();
	}
	_ce->ignore();
 }



void MixerView::setCurrentMixerChannel(int channel)
{
	if (channel >= 0 && channel < m_mixerChannelViews.size())
	{
		setCurrentMixerChannel(m_mixerChannelViews[channel]);
	}
}



void MixerView::clear()
{
	Engine::mixer()->clear();

	refreshDisplay();
}




void MixerView::updateFaders()
{
	Mixer * m = Engine::mixer();

	// apply master gain
	m->mixerChannel(0)->m_peakLeft *= Engine::audioEngine()->masterGain();
	m->mixerChannel(0)->m_peakRight *= Engine::audioEngine()->masterGain();

	for( int i = 0; i < m_mixerChannelViews.size(); ++i )
	{
		const float opl = m_mixerChannelViews[i]->m_fader->getPeak_L();
		const float opr = m_mixerChannelViews[i]->m_fader->getPeak_R();
		const float fallOff = 1.25;
		if( m->mixerChannel(i)->m_peakLeft >= opl/fallOff )
		{
			m_mixerChannelViews[i]->m_fader->setPeak_L( m->mixerChannel(i)->m_peakLeft );
			// Set to -1 so later we'll know if this value has been refreshed yet.
			m->mixerChannel(i)->m_peakLeft = -1;
		}
		else if( m->mixerChannel(i)->m_peakLeft != -1 )
		{
			m_mixerChannelViews[i]->m_fader->setPeak_L( opl/fallOff );
		}

		if( m->mixerChannel(i)->m_peakRight >= opr/fallOff )
		{
			m_mixerChannelViews[i]->m_fader->setPeak_R( m->mixerChannel(i)->m_peakRight );
			// Set to -1 so later we'll know if this value has been refreshed yet.
			m->mixerChannel(i)->m_peakRight = -1;
		}
		else if( m->mixerChannel(i)->m_peakRight != -1 )
		{
			m_mixerChannelViews[i]->m_fader->setPeak_R( opr/fallOff );
		}
	}
}


} // namespace lmms::gui
