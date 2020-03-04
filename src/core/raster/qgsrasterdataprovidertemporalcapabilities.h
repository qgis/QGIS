/***************************************************************************
                         qgsrasterdataprovidertemporalcapabilities.h
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

#ifndef QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
#define QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsdataprovidertemporalcapabilities.h"

/**
 * \class QgsRasterDataProviderTemporalCapabilities
 * \ingroup core
 * Implementation of data provider temporal properties for QgsRasterDataProviders.
 *
 * Data provider temporal capabilities reflect the temporal capabilities of a QgsDataProvider.
 * Unlike QgsMapLayerTemporalProperties, these settings are not user-configurable,
 * and should only be set by the QgsDataProvider itself.
 *
 * \since QGIS 3.14
 */

class CORE_EXPORT QgsRasterDataProviderTemporalCapabilities : public QgsDataProviderTemporalCapabilities
{
  public:

    /**
     * Constructor for QgsRasterDataProviderTemporalProperties.
     */
    QgsRasterDataProviderTemporalCapabilities( bool enabled = false );

    virtual ~QgsRasterDataProviderTemporalCapabilities() = default;

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

    /**
     * Sets the usage status of the reference range.
     *
     * \see isReferenceEnable()
     */
    void setReferenceEnable( bool enabled );

    /**
     * Returns the enabled status of the reference range.
     *
     * \see setReferenceEnable()
    */
    bool isReferenceEnable() const;

  private:

    /**
     * Represents fixed data provider datetime range.
     *
     * This is for determining the providers lower and upper datetime bounds,
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
     * Stores the fixed reference temporal range
     */
    QgsDateTimeRange mFixedReferenceRange;

    //! If these properties has reference temporal range
    bool mHasReferenceRange = false;

    //! If reference range has been enabled to be used in these properties
    bool mReferenceEnable = false;

};

#endif // QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
