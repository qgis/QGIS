/***************************************************************************
                         qgsabstracttemporal.h
                         ---------------
    begin                : January 2020
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


#ifndef QGSABSTRACTTEMPORAL_H
#define QGSABSTRACTTEMPORAL_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsapplication.h"
#include "qgsrange.h"

#include <QObject>
#include <QDateTime>

/**
 * \class QgsAbstractTemporal
 * \ingroup core
 * Base class for temporal based classes.
 *
 * Subclasses may wish to update the abstract temporal functions.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsAbstractTemporal
{
  public:

    /**
     * Constructors for QgsAbstractTemporal.
     */
    QgsAbstractTemporal();

    QgsAbstractTemporal( const bool enabled );

    virtual ~QgsAbstractTemporal() = default;

    /**
     * Sets object as a temporal based one, which will be considered when rendering maps with a specific time range set.
     *
     * \see isTemporal()
     * \since QGIS 3.14
     */
    void setIsTemporal( bool enabled );

    /**
     * Returns true if the object is a temporal one, and will be filtered when rendering maps with a specific time range set.
     *
     * For map settings, If false is returned, then any other temporal settings relating to the map will be ignored during rendering.
     *
     * \see setIsTemporal()
     * \since QGIS 3.14
    */
    bool isTemporal() const;

    /**
     * Set datetime range for a temporal object.
     *
     * When set, can be used to filter and request time base objects.
     *
     * \see temporalRange()
     * \since QGIS 3.14
    */
    void setTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns datetime range if object is a temporal object.
     *
     * \see setTemporalRange()
     * \since QGIS 3.14
    */
    const QgsDateTimeRange &temporalRange() const;

    /**
     * Sets current datetime object.
     *
     * Can be used in map canvas, setting the map temporal instant
     *
     * \see currentDateTime()
     * \since QGIS 3.14
    */

    void setCurrentDateTime( QDateTime *dateTime );

    /**
     * Returns current datetime object.
     *
     * Can be used in map canvas when changing map snapshots
     *
     * \see setCurrentDateTime()
     * \since QGIS 3.14
    */

    QDateTime *currentDateTime() const;

  private:

    //! Temporal state
    bool mTemporal = false;

    //! Represents datetime range member.
    QgsDateTimeRange mDateTimeRange;

    //! Datetime member, for storing instant of time
    QDateTime *mDateTime = nullptr;

};

#endif // QGSABSTRACTTEMPORAL_H
