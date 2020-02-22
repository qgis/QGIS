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

class QgsRenderContext;

/**
 * \class QgsTemporalRangeObject
 * \ingroup core
 * Base class for temporal based classes.
 *
 * Subclasses may wish to update the abstract temporal functions.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsTemporalRangeObject
{
  public:

    /**
     * Constructor for QgsTemporalRangeObject.
     */
    QgsTemporalRangeObject();

    /**
     * Creates temporal range object with the specified temporal state.
     */
    QgsTemporalRangeObject( bool enabled );

    virtual ~QgsTemporalRangeObject() = default;

    /**
     * Sets object as a temporal based one, which will be considered when rendering maps with a specific time range set.
     *
     * \see isTemporal()
     */
    void setIsTemporal( bool enabled );

    /**
     * Returns true if the object is a temporal one, and will be filtered when rendering maps with a specific time range set.
     *
     * For map settings, If false is returned, then any other temporal settings relating to the map will be ignored during rendering.
     *
     * \see setIsTemporal()
    */
    bool isTemporal() const;

    /**
     * Set datetime range for a temporal object.
     *
     * It updates object temporal state to true if it was false.
     *
     * When set, can be used to filter and request time base objects.
     *
     * \see temporalRange()
    */
    void setTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns datetime range if object is a temporal object.
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
