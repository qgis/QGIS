/***************************************************************************
                         qgstemporalrangeobject.h
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


#ifndef QGSTEMPORALRANGEOBJECT_H
#define QGSTEMPORALRANGEOBJECT_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"

/**
 * \class QgsTemporalRangeObject
 * \ingroup core
 * \brief Base class for objects with an associated (optional) temporal range.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalRangeObject
{
  public:

    /**
     * Constructor QgsTemporalRangeObject.
     *
     * The \a enabled argument specifies whether the temporal range is initially enabled or not (see isTemporal()).
     */
    QgsTemporalRangeObject( bool enabled = false );

    virtual ~QgsTemporalRangeObject() = default;

    /**
     * Sets whether the temporal range is \a enabled (i.e. whether the object has a temporal range
     * which will be considered when rendering maps with a specific time range set.)
     *
     * \see isTemporal()
     */
    void setIsTemporal( bool enabled );

    /**
     * Returns TRUE if the object's temporal range is enabled, and the object will be filtered when rendering maps with a specific time range set.
     *
     * For map settings, if FALSE is returned, then any other temporal settings relating to the map will be ignored during rendering.
     *
     * \see setIsTemporal()
    */
    bool isTemporal() const;

    /**
     * Sets the temporal \a range for the object.
     *
     * Calling setTemporalRange() automatically enables temporal properties on the
     * object (see isTemporal()), regardless of its existing state.
     *
     * When a temporal \a range is set it can be used to filter and request time base objects.
     *
     * \see temporalRange()
    */
    void setTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the datetime range for the object.
     *
     * This should only be considered when isTemporal() returns TRUE.
     *
     * \see setTemporalRange()
    */
    const QgsDateTimeRange &temporalRange() const;

  private:

    //! Temporal state
    bool mTemporal = false;

    //! Represents datetime range member.
    QgsDateTimeRange mDateTimeRange;

};

#endif // QGSTEMPORALRANGEOBJECT_H
