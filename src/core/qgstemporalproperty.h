/***************************************************************************
                         qgstemporalproperty.h
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

#ifndef QGSTEMPORALPROPERTY_H
#define QGSTEMPORALPROPERTY_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"


/**
 * \class QgsTemporalProperty
 * \ingroup core
 * Base class for temporal property.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsTemporalProperty
{
  public:

    /**
     * Constructor for QgsTemporalProperty.
     */
    QgsTemporalProperty();

    /**
     * Creates temporal property with the specified temporal state.
     */
    QgsTemporalProperty( const QgsDateTimeRange dateTimeRange );

    /**
     * Initialize the temporal property with datetime instant.
     */
    QgsTemporalProperty( const QDateTime dateTime );

    /**
     * Initialize the temporal property with date instant.
     */
    QgsTemporalProperty( const QDate date );


    /**
     * Copy constructor
     */
    QgsTemporalProperty( const QgsTemporalProperty &temporalProperty );

    virtual ~QgsTemporalProperty() = default;

    /**
     * Set datetime range for the temporal property.
     *
     * \see temporalRange()
    */
    void setTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns datetime range for the temporal property.
     *
     * \see setTemporalRange()
    */
    const QgsDateTimeRange &temporalRange() const;

    /**
     * Set datetime for the temporal property.
     *
     * \see temporalDateTimeInstant()
    */
    void setTemporalDateTimeInstant( const QDateTime &dateTime );

    /**
     * Returns datetime instant for the temporal property.
     *
     * \see setTemporalDateTimeInstant()
    */
    const QDateTime& temporalDateTimeInstant() const;

    /**
     * Set date for the temporal property.
     *
     * \see temporalDateInstant()
    */
    void setTemporalDateInstant( const QDate &date );

    /**
     * Returns date instant for the temporal property.
     *
     * \see setTemporalDateInstant()
    */
    const QDate& temporalDateInstant() const;

  private:

    //! Represents datetime range member.
    QgsDateTimeRange mDateTimeRange;

    //! Datetime instant
    QDateTime mDateTime;

    //! Date instant
    QDate mDate;

};

#endif // QGSTEMPORALPROPERTY_H
