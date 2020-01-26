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
    QgsTemporalProperty( bool enabled = true );

    /**
     * Creates temporal property with the specified temporal state.
     */
    QgsTemporalProperty( const QgsDateTimeRange dateTimeRange, bool enabled = true  );

    /**
     * Initialize the temporal property with datetime instant.
     */
    QgsTemporalProperty( const QDateTime dateTime, bool enabled = true  );

    /**
     * Initialize the temporal property with date instant.
     */
    QgsTemporalProperty( const QDate date, bool enabled = true  );

    /**
     * Initialize temporal property with ISO 8601 date string
     *
     * This will support initializing WMS-T layers with right temporal property range
     *
     *
     */
    QgsTemporalProperty( const QString date, bool enabled = true );

    virtual ~QgsTemporalProperty() = default;

    /**
     * Compare temporal property objects
     */
    bool equal(const QgsTemporalProperty &temporalProperty ) const;

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

    /**
     * Sets temporal property status.
     *
     * \see isActive()
     */
    void setIsActive( bool enabled );

    /**
     * Returns true temporal property status.
     * \see setIsActive()
    */
    bool isActive() const;

    /**
     * Sets temporal property status.
     *
     * \see isActive()
     */
    void setIsActive( bool enabled );

    /**
     * Returns true temporal property status.
     * \see setIsActive()
    */
    bool isActive() const;

  private:

    /**
     * Parse the given string date into one of either date instant or range.
     *
    */
    void parseDate( QString date );


    //! \brief Represents datetime range member.
    QgsDateTimeRange mDateTimeRange;

    //! \brief Datetime instant
    QDateTime mDateTime;

    //! \brief Date instant
    QDate mDate;

    //! \brief Stores temporal status
    bool mActive = false;

    //! \brief Stores temporal status
    bool mActive = false;

};

#endif // QGSTEMPORALPROPERTY_H
