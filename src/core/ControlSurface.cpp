/*
 * ControlSurface.cpp - Common control surface actions to lmms
 *
 * Copyright (c) 2025 - altrouge
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

#include "ControlSurface.h"

#include "Engine.h"
#include "GuiApplication.h"
#include "PianoRoll.h"
#include "Song.h"

namespace lmms {
void ControlSurface::play()
{
	Engine::getSong()->playSong();
}

void ControlSurface::stop()
{
	auto piano_roll = gui::getGUI()->pianoRoll();
	if (piano_roll != nullptr) { piano_roll->stop(); }
	Engine::getSong()->stop();
}

void ControlSurface::loop()
{
	// Activate on MidiClip for piano roll and Song for whole song.
	auto& timeline_midi = Engine::getSong()->getTimeline(Song::PlayMode::MidiClip);
	timeline_midi.setLoopEnabled(!timeline_midi.loopEnabled());
	auto& timeline = Engine::getSong()->getTimeline(Song::PlayMode::Song);
	timeline.setLoopEnabled(!timeline.loopEnabled());
}

void ControlSurface::record()
{
	auto piano_roll = gui::getGUI()->pianoRoll();
	if (piano_roll != nullptr) { piano_roll->record(); }
}
} // namespace lmms
