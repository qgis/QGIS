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
#include "qgstemporalcontroller.h"
#include "qgsexpressioncontextscopegenerator.h"

#include <QList>
#include <QTimer>

class QgsMapLayer;

/**
 * \ingroup core
 * \brief Implements a temporal controller based on a frame by frame navigation and animation.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalNavigationObject : public QgsTemporalController, public QgsExpressionContextScopeGenerator
{
    Q_OBJECT

  public:

    /**
      * Constructor for QgsTemporalNavigationObject, with the specified \a parent object.
      */
    QgsTemporalNavigationObject( QObject *parent SIP_TRANSFERTHIS = nullptr );

    //! Represents the current temporal navigation mode.
    enum NavigationMode
    {
      NavigationOff, //!< Temporal navigation is disabled
      Animated, //!< Temporal navigation relies on frames within a datetime range
      FixedRange, //!< Temporal navigation relies on a fixed datetime range
    };

    //! Represents the current animation state.
    enum AnimationState
    {
      Forward, //!< Animation is playing forward.
      Reverse, //!< Animation is playing in reverse.
      Idle, //!< Animation is paused.
    };

    /**
     * Sets the current animation \a state.
     *
     * \see animationState()
     */
    void setAnimationState( AnimationState state );

    /**
     * Returns the current animation state.
     *
     * \see setAnimationState()
     */
    AnimationState animationState() const;

    /**
     * Sets the temporal navigation \a mode.
     *
     * \see navigationMode()
     */
    void setNavigationMode( const NavigationMode mode );

    /**
     * Returns the current temporal navigation mode.
     *
     * \see setNavigationMode()
     */
    NavigationMode navigationMode() const { return mNavigationMode; }

    /**
     * Sets the navigation temporal \a extents, which dictate the earliest
     * and latest date time possible in the animation.
     *
     * \note Calling this will reset the currentFrameNumber() to the first frame.
     *
     * \see temporalExtents()
     */
    void setTemporalExtents( const QgsDateTimeRange &extents );

    /**
     * Returns the navigation temporal extents, which dictate the earliest
     * and latest date time possible in the animation.
     *
     * \see setTemporalExtents()
     */
    QgsDateTimeRange temporalExtents() const;

    /**
     * Sets the list of all available temporal \a ranges which have data available.
     *
     * The \a ranges list can be a list of non-contiguous ranges (i.e. containing gaps)
     * which together describe the complete range of times which contain data.
     *
     * \see availableTemporalRanges()
     * \since QGIS 3.20
     */
    void setAvailableTemporalRanges( const QList< QgsDateTimeRange > &ranges );

    /**
     * Returns the list of all available temporal ranges which have data available.
     *
     * The ranges list can be a list of non-contiguous ranges (i.e. containing gaps)
     * which together describe the complete range of times which contain data.
     *
     * \see setAvailableTemporalRanges()
     * \since QGIS 3.20
     */
    QList< QgsDateTimeRange > availableTemporalRanges() const;

    /**
     * Sets the current animation \a frame number.
     *
     * Calling this method will change the controllers current datetime range to match,
     * based on the temporalExtents() and frameDuration() values.
     *
     * \see currentFrameNumber()
     */
    void setCurrentFrameNumber( long long frame );

    /**
     * Returns the current frame number.
     *
     * \see setCurrentFrameNumber()
     */
    long long currentFrameNumber() const;

    /**
     * Sets the frame \a duration, which dictates the temporal length of each frame in the animation.
     *
     * \note Calling this will reset the currentFrameNumber() to the closest temporal match for the previous temporal range.
     *
     * \see frameDuration()
     */
    void setFrameDuration( const QgsInterval &duration );

    /**
     * Returns the current set frame duration, which dictates the temporal length of each frame in the animation.
     *
     * \see setFrameDuration()
     */
    QgsInterval frameDuration() const;

    /**
     * Calculates the temporal range associated with a particular animation \a frame.
     *
     * This is calculated from the navigation start time (taken from temporalExtents()),
     * the specified \a frame number, and the frame duration (see frameDuration()).
     */
    QgsDateTimeRange dateTimeRangeForFrameNumber( long long frame ) const;

    /**
     * Sets the animation frame \a rate, in frames per second.
     *
     * This setting controls the overall playback speed of the animation, i.e. how quickly
     * a playing animation will advance to the next frame.
     *
     * \see framesPerSecond()
     */
    void setFramesPerSecond( double rate );

    /**
     * Returns the animation frame rate, in frames per second.
     *
     * This setting controls the overall playback speed of the animation, i.e. how quickly
     * a playing animation will advance to the next frame.
     *
     * \see setFramesPerSecond()
     */
    double framesPerSecond() const;

    /**
     * Sets the animation temporal range as cumulative.
     *
     * \see temporalRangeCumulative()
     */
    void setTemporalRangeCumulative( bool state );

    /**
     * Returns the animation temporal range cumulative settings.
     *
     * \see setTemporalRangeCumulative()
     */
    bool temporalRangeCumulative() const;

    /**
     * Returns the total number of frames for the navigation.
     */
    long long totalFrameCount() const;

    /**
     * Returns TRUE if the animation should loop after hitting the end or start frame.
     *
     * \see setLooping()
     */
    bool isLooping() const;

    /**
     * Sets whether the animation should \a loop after hitting the end or start frame.
     *
     * \see isLooping()
     */
    void setLooping( bool loop );

    /**
     * Returns the best suited frame number for the specified datetime, based on the start of the corresponding temporal range.
     */
    long long findBestFrameNumberForFrameStart( const QDateTime &frameStart ) const;

    QgsExpressionContextScope *createExpressionContextScope() const override SIP_FACTORY;

  signals:

    /**
     * Emitted whenever the animation \a state changes.
     */
    void stateChanged( QgsTemporalNavigationObject::AnimationState state );

    /**
     * Emitted whenever the navigation \a mode changes.
     */
    void navigationModeChanged( QgsTemporalNavigationObject::NavigationMode mode );

    /**
     * Emitted whenever the temporalExtent \a extent changes.
     */
    void temporalExtentsChanged( const QgsDateTimeRange &extent );

    /**
     * Emitted whenever the frameDuration \a interval of the controller changes.
     */
    void temporalFrameDurationChanged( const QgsInterval &interval );


  public slots:

    /**
     * Starts playing the temporal navigation from its current frame,
     * using the direction specified by animationState()
     */
    void play();

    /**
     * Pauses the temporal navigation.
     *
     * Calling this slot changes the animation state to idle, preventing
     * automatic advancement of frames.
     *
     * It does not affect the current animation frame number or the current
     * temporal range of the controller.
     */
    void pause();

    /**
     * Starts the animation playing in a forward direction up till the end of all frames.
     */
    void playForward();

    /**
     * Starts the animation playing in a reverse direction until the beginning of the time range.
     */
    void playBackward();

    /**
     * Advances to the next frame.
     *
     * \note Calling this slot does not change the current animation state, i.e. a paused animation
     * will remain paused.
     */
    void next();

    /**
     * Jumps back to the previous frame.
     *
     * \note Calling this slot does not change the current animation state, i.e. a paused animation
     * will remain paused.
     */
    void previous();

    /**
     * Rewinds the temporal navigation to start of the temporal extent.
     */
    void rewindToStart();

    /**
     * Skips the temporal navigation to end of the temporal extent.
     */
    void skipToEnd();

  private slots:

    //! Handles logic when the temporal navigation timer emit a timeout signal.
    void timerTimeout();

  private:

    //! The controller temporal navigation extent range.
    QgsDateTimeRange mTemporalExtents;

    //! Complete list of time ranges with data available
    QList< QgsDateTimeRange > mAllRanges;

    NavigationMode mNavigationMode = NavigationOff;

    //! The current set frame value
    long long mCurrentFrameNumber = 0;

    //! Frame duration
    QgsInterval mFrameDuration = QgsInterval( 1.0, QgsUnitTypes::TemporalUnit::TemporalHours );

    //! Member for frame rate
    double mFramesPerSecond = 1;

    //! Timer to set navigation time interval
    QTimer *mNewFrameTimer = nullptr;

    //! Navigation playback mode member
    AnimationState mPlayBackMode = Idle;

    bool mLoopAnimation = false;

    bool mCumulativeTemporalRange = false;

    int mBlockUpdateTemporalRangeSignal = 0;

    QgsTemporalNavigationObject( const QgsTemporalNavigationObject & ) = delete;
    QgsTemporalNavigationObject &operator= ( const QgsTemporalNavigationObject & ) = delete;
};

#endif // QGSTEMPORALNAVIGATIONOBJECT_H
