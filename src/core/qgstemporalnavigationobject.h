/***************************************************************************
                         qgstemporalnavigationobject.h
                         ---------------
    begin                : March 2020
    copyright            : (C) 2020 by Samweli Mwakisambwe
    email                : samweli at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSTEMPORALNAVIGATIONOBJECT_H
#define QGSTEMPORALNAVIGATIONOBJECT_H

#include "qgis_core.h"
#include "qgsrange.h"
#include "qgsinterval.h"

#include <QList>
#include <QTimer>

class QgsMapLayer;

/**
 * \ingroup core
 * The QgsTemporalNavigationObject class
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalNavigationObject : public QObject
{
    Q_OBJECT

  public:

    /**
      * Constructor for QgsTemporalNavigationObject
      *
      */
    QgsTemporalNavigationObject( QObject *parent = nullptr );

    //! Represents the current playback mode
    enum PlaybackMode
    {
      Forward, //! When the forward button is clicked.
      Reverse, //! When the back button is clicked.
      Idle, //! When navigation is idle.
    };

    /**
     * Sets the current playback mode.
     *
     * \see playBackMode()
     */
    void setPlayBackMode( PlaybackMode mode );

    /**
     * Returns the current playback mode.
     *
     * \see setPlayBackMode()
     */
    PlaybackMode playBackMode() const;

    /**
     * Sets the navigation temporal extents.
     *
     * \see temporalExtents()
     */
    void setTemporalExtents( QgsDateTimeRange temporalExtents );

    /**
     * Returns the navigation temporal extent.
     *
     * \see setTemporalExtents()
     */
    QgsDateTimeRange temporalExtents() const;

    /**
     * Sets the current frame value.
     *
     * \see currentFrameNumber()
     */
    void setCurrentFrameNumber( long long frameNumber );

    /**
     * Returns the current set frame value.
     *
     * \see setCurrentFrameNumber();
     */
    long long currentFrameNumber() const;

    /**
     * Sets the frame duration
     *
     * \see frameDuration()
     */
    void setFrameDuration( QgsInterval frameDuration );

    /**
     * Returns the current set frame value.
     *
     * \see setFrameDuration()
     */
    QgsInterval frameDuration() const;

    /**
     * Calculate the temporal range from the navigation start time, using
     * current frame number and the frame duration.
     *
     */
    QgsDateTimeRange dateTimeRangeForFrameNumber( long long frame ) const;

    /**
     * Sets the frames per seconds value. This is used to define
     * the navigation frame rate.
     *
     * \see framesPerSeconds()
     */
    void setFramesPerSeconds( double framesPerSeconds );

    /**
     * Returns the set frames per seconds value.
     *
     * \see setFramesPerSeconds();
     */
    double framesPerSeconds() const;

    /**
     * Returns the total number of frames for the navigation.
     *
     */
    long long totalFrameCount();

  public slots:

    //! Plays the temporal navigation
    void play();

    //! Stops the temporal navigation.
    void pause();

    //! Forward the temporal navigation up till the end of frames.
    void forward();

    //! Decrement the temporal navigation till the end of frames.
    void backward();

    //! Forward the temporal navigation by one frame.
    void next();

    //! Decrement the temporal navigation by one frame.
    void previous();

    //! Rewind the temporal navigation to start of the temporal extent.
    void rewindToStart();

    //! Skips the temporal navigation to end of the temporal extent.
    void skipToEnd();

    //! Handles logic when the temporal navigation timer emit a timeout signal.
    void timerTimeout();

  signals:

    //! Emitted when navigation range changes.
    void updateTemporalRange( QgsDateTimeRange temporalRange );

  private:

    //! The controller temporal navigation extent range.
    QgsDateTimeRange mTemporalExtents;

    //! The current set frame value
    long long mCurrentFrameNumber = 0;

    //! Frame duration
    QgsInterval mFrameDuration;

    //! Member for frame rate
    double mFramesPerSecond;

    //! Timer to set navigation time interval
    QTimer *mNewFrameTimer;

    //! Navigation playback mode member
    PlaybackMode mPlayBackMode = Idle;

};

#endif // QGSTEMPORALNAVIGATIONOBJECT_H
