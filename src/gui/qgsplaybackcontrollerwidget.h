/***************************************************************************
   qgsplaybackcontrollerwidget.h
    --------------------------------------
    begin                : November 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSPLAYBACKCONTROLLERWIDGET_H
#define QGSPLAYBACKCONTROLLERWIDGET_H

#include "ui_qgsplaybackcontrollerwidgetbase.h"
#include "qgis_sip.h"
#include "qgis_gui.h"
#include "qgis.h"
#include <QWidget>

/**
 * \ingroup gui
 * \brief A compound widget containing a set of buttons for controlling media playback.
 * \since QGIS 3.36
 */
class GUI_EXPORT QgsPlaybackControllerWidget : public QWidget, private Ui::QgsPlaybackControllerWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPlaybackControllerWidget, with the specified \a parent widget.
     */
    explicit QgsPlaybackControllerWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the button associated with the specified playback \a operation.
     */
    QPushButton *button( Qgis::PlaybackOperation operation );

  public slots:

    /**
     * Sets the current animation \a state for the widget.
     */
    void setState( Qgis::AnimationState state );

    /**
     * Toggles the pause state on or off.
     */
    void togglePause();

  signals:

    /**
     * Emitted when a playback operation is triggered.
     */
    void operationTriggered( Qgis::PlaybackOperation operation );

  private slots:

    void togglePlayForward();
    void togglePlayBackward();
    void pause();
    void next();
    void previous();
    void skipToEnd();
    void rewindToStart();

  private:
    Qgis::AnimationState mAnimationState = Qgis::AnimationState::Idle;
    bool mPlayingForward = true;
};

#endif // QGSPLAYBACKCONTROLLERWIDGET_H
