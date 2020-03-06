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

#include "qgsrange.h"
#include <QList>
#include "qgstemporalcontroller.h"

class QgsMapLayer;

/**
 * \ingroup core
 * The QgsTemporalNavigationObject class
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalNavigationObject : public QgsTemporalController
{

  public:

    /**
      * Constructor for QgsTemporalNavigationObject
      *
      */
    QgsTemporalNavigationObject();

    //! Represents the current VCR navigation status
    enum NavigationStatus
    {
      Forward, //! When the forward button is clicked
      BackWard, //! When the back button is clicked
      Next, //! When the next button is clicked
      Previous, //! When the previous button is clicked
      Idle //! When no button has been clicked
    };

    //! Represents the VCR modes in getting data from layers
    enum Mode
    {
      NearestPreviousProduct, //! Get the nearest previous data if the requested one is not available
      Snapshot, //! Return the layer data which match exactly the datetime value in the request
      Composite //! Get the range of datetimes, using the specified time steps
    };

    /**
     * Sets the current VCR widget navigation status.
     *
     * \see navigationStatus()
     */
    void setNavigationStatus( NavigationStatus status );

    /**
     * Returns the current VCR widget navigation status.
     *
     * \see setNavigationStatus()
     */
    NavigationStatus navigationStatus() const;

    /**
     * Sets the current VCR mode.
     *
     * \see mode()
     */
    void setMode( Mode mode );

    /**
     * Returns the current VCR mode.
     *
     * \see setMode()
     */
    Mode mode() const;

    /**
     * Sets the play status.
     *
     * \see isPlaying()
     */
    void setIsPlaying( bool playing );

    /**
     * Returns the play status.
     *
     * \see setIsPlaying()
     */
    bool isPlaying() const;

    /**
     * Updates the project temporal layers with the given \a datetime.
     */
    void updateLayersTemporalRange( QDateTime dateTime, QString time, int value );

    /**
     * Returns the datetime after adding time value to the passed \a datetime.
     */
    QDateTime addToDateTime( QDateTime dateTime, QString time, int value );

    /**
     * Returns the calculated temporal range from the current mode.
     */
    QgsDateTimeRange rangeFromMode( QgsMapLayer *layer, QDateTime dateTime, QString time, int value );

    /**
     * Returns the nearest datetime from the list of \a datetimes that is less than
     * the \a datetime.
     */
    QDateTime lessNearestDateTime( QList<QDateTime> dateTimes, QDateTime dateTime );

    /**
     * Returns the list of datetimes, which VCR widget will navigate upon.
     *
     * \see setDateTimes()
     */
    QList<QDateTime> dateTimes() const;

    /**
     * Sets the list of datetimes, which VCR widget will use to update the
     * temporal layers.
     *
     * \see dateTimes()
     */
    void setDateTimes( QList<QDateTime> dateTimes );

  private:

    //! Holds the list of datetimes to navigate.
    QList <QDateTime> mDateTimes;

    //! Navigation status.
    NavigationStatus mStatus = Idle;

    //! VCR mode.
    Mode mMode = Snapshot;

    //! Whether navigation is in play.
    bool mPlayActive = false;

};

#endif // QGSTEMPORALNAVIGATIONOBJECT_H
