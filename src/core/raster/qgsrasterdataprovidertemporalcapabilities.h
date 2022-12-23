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
#include "qgsinterval.h"
#include "qgsdataprovidertemporalcapabilities.h"

/**
 * \class QgsRasterDataProviderTemporalCapabilities
 * \ingroup core
 * \brief Implementation of data provider temporal properties for QgsRasterDataProviders.
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
     *
     * The \a enabled argument specifies whether the data provider has temporal capabilities.
     */
    QgsRasterDataProviderTemporalCapabilities( bool enabled = false );

    /**
     * Returns the capability flags for the provider.
     *
     * \since QGIS 3.28
     */
    Qgis::RasterTemporalCapabilityFlags flags() const;

    /**
     * Sets the capability flags for the provider.
     *
     * \since QGIS 3.28
     */
    void setFlags( Qgis::RasterTemporalCapabilityFlags flags );

    /**
     * Returns the desired method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see setIntervalHandlingMethod()
    */
    Qgis::TemporalIntervalMatchMethod intervalHandlingMethod() const;

    /**
     * Sets the desired \a method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see intervalHandlingMethod()
    */
    void setIntervalHandlingMethod( Qgis::TemporalIntervalMatchMethod method );

    /**
     * Sets the overall datetime \a range extent from which temporal data is available from the provider.
     *
     * \see availableTemporalRange()
    */
    void setAvailableTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the overall datetime range extent from which temporal data is available from the provider.
     *
     * \see setAvailableTemporalRange()
    */
    const QgsDateTimeRange &availableTemporalRange() const;

    /**
     * Sets a list of all valid datetime \a ranges for which temporal data is available from the provider.
     *
     * As opposed to setAvailableTemporalRange(), this method is useful when a provider
     * contains a set of non-contiguous datetime ranges.
     *
     * \see allAvailableTemporalRanges()
     * \see setAvailableTemporalRange()
     * \since QGIS 3.20
    */
    void setAllAvailableTemporalRanges( const QList< QgsDateTimeRange > &ranges );

    /**
     * Returns a list of all valid datetime ranges for which temporal data is available from the provider.
     *
     * As opposed to availableTemporalRange(), this method is useful when a provider
     * contains a set of non-contiguous datetime ranges.
     *
     * \see setAllAvailableTemporalRanges()
     * \see availableTemporalRange()
     * \since QGIS 3.20
    */
    QList< QgsDateTimeRange > allAvailableTemporalRanges() const;

    /**
     * Sets the available reference datetime \a range. This is to be used for
     * bi-temporal based data. Where data can have both nominal and reference times.
     *
     * \see availableReferenceTemporalRange()
    */
    void setAvailableReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the available reference datetime range, which indicates the maximum
     * extent of datetime values available for reference temporal ranges from the provider.
     *
     * \see setAvailableReferenceTemporalRange()
    */
    const QgsDateTimeRange &availableReferenceTemporalRange() const;

    /**
     * Returns the requested temporal range.
     * Intended to be used by the provider in fetching data.
    */
    const QgsDateTimeRange &requestedTemporalRange() const;

    /**
     * Returns the default time step interval corresponding to the available
     * datetime values for the provider.
     *
     * \see setDefaultInterval()
     * \since QGIS 3.20
     */
    QgsInterval defaultInterval() const;

    /**
     * Sets the default time step \a interval corresponding to the available
     * datetime values for the provider.
     *
     * \see defaultInterval()
     * \since QGIS 3.20
     */
    void setDefaultInterval( const QgsInterval &interval );

    /**
     * Sets the requested temporal \a range to retrieve when
     * returning data from the associated data provider.
     *
     * \note this is not normally manually set, and is intended for use by
     * QgsRasterLayerRenderer to automatically set the requested temporal range
     * on a clone of the data provider during a render job.
     *
     * \see requestedTemporalRange()
    */
    void setRequestedTemporalRange( const QgsDateTimeRange &range ) SIP_SKIP;

  private:

    /**
     * Represents available data provider datetime range.
     *
     * This is for determining the providers lower and upper datetime bounds,
     * any updates on the mRange should get out the range bound defined
     * by this member.
     *
     */
    QgsDateTimeRange mAvailableTemporalRange;

    /**
     * A list of all valid temporal ranges for the provider. Used when a provider
     * has a non-contiguous set of available temporal ranges.
     */
    QList< QgsDateTimeRange > mAllAvailableTemporalRanges;

    //! Represents the requested temporal range.
    QgsDateTimeRange mRequestedRange;

    /**
     * Stores the available reference temporal range
     */
    QgsDateTimeRange mAvailableReferenceRange;

    QgsInterval mDefaultInterval;

    //! Interval handling method
    Qgis::TemporalIntervalMatchMethod mIntervalMatchMethod = Qgis::TemporalIntervalMatchMethod::MatchUsingWholeRange;

    Qgis::RasterTemporalCapabilityFlags mFlags;

    friend class QgsRasterLayerRenderer;
    friend class TestQgsRasterDataProviderTemporalCapabilities;

};

#endif // QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
