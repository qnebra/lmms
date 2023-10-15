/*
 * AutomationEditor.h - declaration of class AutomationEditor which is a window
 *					  where you can edit dynamic values in an easy way
 *
 * Copyright (c) 2006-2008 Javier Serrano Polo <jasp00/at/users.sourceforge.net>
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

#ifndef LMMS_GUI_AUTOMATION_EDITOR_H
#define LMMS_GUI_AUTOMATION_EDITOR_H

#include <QPushButton>
#include <QWidget>
#include <array>

#include "AutomationClip.h"
#include "ComboBoxModel.h"
#include "Editor.h"
#include "JournallingObject.h"
#include "MidiClip.h"
#include "TimePos.h"
#include "lmms_basics.h"

class QPainter;
class QPixmap;
class QScrollBar;

namespace lmms
{

class NotePlayHandle;

namespace gui
{

class Knob;
class ComboBox;
class TimeLineWidget;



class AutomationEditor : public QWidget, public JournallingObject
{
	Q_OBJECT
	Q_PROPERTY(QColor barLineColor MEMBER m_barLineColor)
	Q_PROPERTY(QColor beatLineColor MEMBER m_beatLineColor)
	Q_PROPERTY(QColor lineColor MEMBER m_lineColor)
	Q_PROPERTY(QColor nodeInValueColor MEMBER m_nodeInValueColor)
	Q_PROPERTY(QColor nodeOutValueColor MEMBER m_nodeOutValueColor)
	Q_PROPERTY(QColor nodeTangentLineColor MEMBER m_nodeTangentLineColor)
	Q_PROPERTY(QBrush scaleColor MEMBER m_scaleColor)
	Q_PROPERTY(QBrush graphColor MEMBER m_graphColor)
	Q_PROPERTY(QColor crossColor MEMBER m_crossColor)
	Q_PROPERTY(QColor backgroundShade MEMBER m_backgroundShade)
public:
	void setCurrentClip(AutomationClip * new_clip);
	void setGhostMidiClip(MidiClip* newMidiClip);

	inline const AutomationClip * currentClip() const
	{
		return m_clip;
	}

	inline bool validClip() const
	{
		return m_clip != nullptr;
	}

	void saveSettings(QDomDocument & doc, QDomElement & parent) override;
	void loadSettings(const QDomElement & parent) override;
	QString nodeName() const override
	{
		return "automationeditor";
	}

	enum class EditMode
	{
		Draw,
		Erase,
		DrawOutValues,
		EditTangents
	};

public slots:
	void update();
	void updateAfterClipChange();


protected:
	using timeMap = AutomationClip::timeMap;

	void keyPressEvent(QKeyEvent * ke) override;
	void leaveEvent(QEvent * e) override;
	void mousePressEvent(QMouseEvent * mouseEvent) override;
	void mouseDoubleClickEvent(QMouseEvent * mouseEvent) override;
	void mouseReleaseEvent(QMouseEvent * mouseEvent) override;
	void mouseMoveEvent(QMouseEvent * mouseEvent) override;
	void paintEvent(QPaintEvent * pe) override;
	void resizeEvent(QResizeEvent * re) override;
	void wheelEvent(QWheelEvent * we) override;

	float getLevel( int y );
	int xCoordOfTick( int tick );
	float yCoordOfLevel( float level );
	inline void drawLevelTick(QPainter & p, int tick, float value);

	timeMap::iterator getNodeAt(int x, int y, bool outValue = false, int r = 5);
	/**
	 * @brief Given a mouse X coordinate, returns a timeMap::iterator that points to
	 *        the closest node.
	 * @param Int X coordinate
	 * @return timeMap::iterator with the closest node or timeMap.end() if there are no nodes.
	 */
	timeMap::iterator getClosestNode(int x);

	void drawLine( int x0, float y0, int x1, float y1 );
	bool fineTuneValue(timeMap::iterator node, bool editingOutValue);

protected slots:
	void play();
	void stop();

	void horScrolled( int new_pos );
	void verScrolled( int new_pos );

	void setEditMode(AutomationEditor::EditMode mode);
	void setEditMode(int mode);

	void setProgressionType(AutomationClip::ProgressionType type);
	/**
	 * @brief This method handles the AutomationEditorWindow event of changing
	 * progression types. After that, it calls updateEditTanButton so the edit
	 * tangents button is updated accordingly
	 * @param Int New progression type
	 */
	void setProgressionType(int type);
	void setTension();

	void updatePosition( const lmms::TimePos & t );

	void zoomingXChanged();
	void zoomingYChanged();

	/// Updates the clip's quantization using the current user selected value.
	void setQuantization();

	void resetGhostNotes()
	{
		m_ghostNotes = nullptr;
		update();
	}

private:

	enum class Action
	{
		None,
		MoveValue,
		EraseValues,
		MoveOutValue,
		ResetOutValues,
		DrawLine,
		MoveTangent,
		ResetTangents
	} ;

	// some constants...
	static const int SCROLLBAR_SIZE = 12;
	static const int TOP_MARGIN = 16;

	static const int DEFAULT_Y_DELTA = 6;
	static const int DEFAULT_STEPS_PER_BAR = 16;
	static const int DEFAULT_PPB = 12 * DEFAULT_STEPS_PER_BAR;

	static const int VALUES_WIDTH = 64;

	AutomationEditor();
	AutomationEditor( const AutomationEditor & );
	~AutomationEditor() override;

	static QPixmap * s_toolDraw;
	static QPixmap * s_toolErase;
	static QPixmap * s_toolDrawOut;
	static QPixmap * s_toolEditTangents;
	static QPixmap * s_toolMove;
	static QPixmap * s_toolYFlip;
	static QPixmap * s_toolXFlip;

	ComboBoxModel m_zoomingXModel;
	ComboBoxModel m_zoomingYModel;
	ComboBoxModel m_quantizeModel;

	static const std::array<float, 7> m_zoomXLevels;

	FloatModel * m_tensionModel;

	AutomationClip * m_clip;
	float m_minLevel;
	float m_maxLevel;
	float m_step;
	float m_scrollLevel;
	float m_bottomLevel;
	float m_topLevel;

	MidiClip* m_ghostNotes;

	void centerTopBottomScroll();
	void updateTopBottomLevels();

	QScrollBar * m_leftRightScroll;
	QScrollBar * m_topBottomScroll;

	TimePos m_currentPosition;

	Action m_action;

	int m_moveXOffset;

	float m_drawLastLevel;
	tick_t m_drawLastTick;

	int m_ppb;
	int m_y_delta;
	bool m_y_auto;

	// Time position (key) of automation node whose outValue is being dragged
	int m_draggedOutValueKey;

	// The tick from the node whose tangent is being dragged
	int m_draggedTangentTick;
	// Whether the tangent being dragged is the InTangent or OutTangent
	bool m_draggedOutTangent;

	EditMode m_editMode;

	bool m_mouseDownLeft;
	bool m_mouseDownRight; //true if right click is being held down

	TimeLineWidget * m_timeLine;
	bool m_scrollBack;

	void drawCross(QPainter & p );
	void drawAutomationPoint( QPainter & p, timeMap::iterator it );
	void drawAutomationTangents(QPainter& p, timeMap::iterator it);
	bool inPatternEditor();

	QColor m_barLineColor;
	QColor m_beatLineColor;
	QColor m_lineColor;
	QBrush m_graphColor;
	QColor m_nodeInValueColor;
	QColor m_nodeOutValueColor;
	QColor m_nodeTangentLineColor;
	QBrush m_scaleColor;
	QColor m_crossColor;
	QColor m_backgroundShade;

	friend class AutomationEditorWindow;


signals:
	void currentClipChanged();
	void positionChanged( const lmms::TimePos & );
} ;




class AutomationEditorWindow : public Editor
{
	Q_OBJECT

	static const int INITIAL_WIDTH = 860;
	static const int INITIAL_HEIGHT = 480;
public:
	AutomationEditorWindow();
	~AutomationEditorWindow() override = default;

	void setCurrentClip(AutomationClip* clip);
	void setGhostMidiClip(MidiClip* clip) { m_editor->setGhostMidiClip(clip); };

	const AutomationClip* currentClip();

	void dropEvent( QDropEvent * _de ) override;
	void dragEnterEvent( QDragEnterEvent * _dee ) override;

	void open(AutomationClip* clip);

	AutomationEditor* m_editor;

	QSize sizeHint() const override;

public slots:
	void clearCurrentClip();

signals:
	void currentClipChanged();

protected:
	void focusInEvent(QFocusEvent * event) override;

protected slots:
	void play() override;
	void stop() override;

private slots:
	void updateWindowTitle();
	void setProgressionType(int progType);
	/**
	 * @brief The Edit Tangent edit mode should only be available for
	 * Cubic Hermite progressions, so this method is responsable for disabling it
	 * for other edit modes and reenabling it when it changes back to the Edit Tangent
	 * mode.
	 */
	void updateEditTanButton();

private:
	QAction* m_drawAction;
	QAction* m_eraseAction;
	QAction* m_drawOutAction;
	QAction* m_editTanAction;

	QAction* m_discreteAction;
	QAction* m_linearAction;
	QAction* m_cubicHermiteAction;

	QAction* m_flipYAction;
	QAction* m_flipXAction;

	Knob * m_tensionKnob;

	ComboBox * m_zoomingXComboBox;
	ComboBox * m_zoomingYComboBox;
	ComboBox * m_quantizeComboBox;

	QPushButton* m_resetGhostNotes;
};

} // namespace gui

} // namespace lmms

#endif // LMMS_GUI_AUTOMATION_EDITOR_H
