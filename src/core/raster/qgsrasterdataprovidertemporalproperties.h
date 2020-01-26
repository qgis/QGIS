/***************************************************************************
                         qgsrasterdataprovidertemporalproperties.h
                         ---------------
    begin                : February 2020
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

#ifndef QGSRASTERDATAPROVIDERTEMPORALPROPERTIES_H
#define QGSRASTERDATAPROVIDERTEMPORALPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsdataprovidertemporalproperties.h"

/**
 * \class QgsRasterDataProviderTemporalProperties
 * \ingroup core
 * Implementation raster data provider temporal properties.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsRasterDataProviderTemporalProperties : public QgsDataProviderTemporalProperties
{
  public:

    /**
     * Constructor for QgsRasterDataProviderTemporalProperties.
     */
    QgsRasterDataProviderTemporalProperties( bool enabled = false );

    virtual ~QgsRasterDataProviderTemporalProperties() = default;

//    QgsRasterDataProviderTemporalProperties  &operator=( const QgsRasterDataProviderTemporalProperties &other );

    /**
     * Sets the current active datetime range for the temporal properties.
     *
     * \see temporalRange()
    */
    void setTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns the current active datetime range for these temporal properties.
     *
     * \see setTemporalRange()
    */
    const QgsDateTimeRange &temporalRange() const;

    /**
     * Sets the current active reference datetime range for the temporal properties.
     *
     * This will be used by bi-temporal dimensional data providers.
     *
     * \see referenceTemporalRange()
    */
    void setReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns the current active reference datetime range for these temporal properties.
     *
     * \see setReferenceTemporalRange()
    */
    const QgsDateTimeRange &referenceTemporalRange() const;

    /**
     * Sets the fixed datetime range for the temporal properties.
     *
     * \see fixedTemporalRange()
    */
    void setFixedTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns the fixed datetime range for these temporal properties.
     *
     * \see setFixedTemporalRange()
    */
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets the fixed reference datetime range for the temporal properties.
     *
     * \see fixedReferenceTemporalRange()
    */
    void setFixedReferenceTemporalRange( const QgsDateTimeRange &dateTimeRange );

    /**
     * Returns the fixed reference datetime range for these temporal properties.
     *
     * \see setFixedReferenceTemporalRange()
    */
    const QgsDateTimeRange &fixedReferenceTemporalRange() const;

    /**
     * Sets the time enabled status.
     *
     * \see isTimeEnabled()
     */
    void setEnableTime( bool enabled );

    /**
     * Returns the temporal property status.
     *
     * \see setEnableTime()
    */
    bool isTimeEnabled() const;

    /**
     * Sets the reference range status.
     *
     * \see hasReference()
     */
    void setHasReference( bool enabled );

    /**
     * Returns the reference range presence status.
     *
     * \see setHasReference()
    */
    bool hasReference() const;

  private:
    //! Represents current active datetime range member.
    QgsDateTimeRange mRange;

    /**
     * Represents fixed data provider datetime range.
     *
     * This is for determing the providers lower and upper datetime bounds,
     * any updates on the mRange should get out the range bound defined
     * by this member.
     *
     */
    QgsDateTimeRange mFixedRange;

    /**
     * If the stored time part in temporal ranges should be taked into account.
     *
     * This is to enable data providers that use dates only and no datetime, to
     * configure their temporal properties to consider their state.
     *
     * eg. some WMS-T providers only require date with "YYYY-MM-DD" format with
     *  no time part.
     */
    bool mEnableTime = true;

    /**
     * Stores reference temporal range
     */
    QgsDateTimeRange mReferenceRange;

    /**
     * Stores the fixed reference temporal range
     */
    QgsDateTimeRange mFixedReferenceRange;

    //! If these properties has reference temporal range
    bool mHasReferenceRange = false;


};

#endif // QGSRASTERDATAPROVIDERTEMPORALPROPERTIES_H
