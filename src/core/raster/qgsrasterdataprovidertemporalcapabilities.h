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
     *
     * The \a enabled argument specifies whether the data provider has temporal capabilities.
     */
    QgsRasterDataProviderTemporalCapabilities( bool enabled = false );

    virtual ~QgsRasterDataProviderTemporalCapabilities() = default;

    /**
     * Mode of the temporal capabilities
     **/
    enum TemporalMode
    {
      ModeFixedTemporalRange = 0, //!< Mode when temporal capabilities have fixed start and end datetimes.
      ModeTemporalRangeFromDataProvider = 1 //!< Mode when temporal capabilities provides the temporal range .
    };

    /**
     * Returns the temporal mode.
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Sets the temporal \a mode.
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Method to use when resolving a temporal range to a data provider layer or band.
     **/
    enum IntervalHandlingMethod
    {
      MatchUsingWholeRange, //!< Use an exact match to the whole temporal range
      MatchExactUsingStartOfRange, //!< Match the start of the temporal range to a corresponding layer or band, and only use exact matching results
      MatchExactUsingEndOfRange, //!< Match the end of the temporal range to a corresponding layer or band, and only use exact matching results
    };
    // TODO -- add other methods, like "FindClosestMatchToStartOfRange", "FindClosestMatchToEndOfRange", etc

    /**
     * Returns the desired method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see setIntervalHandlingMethod()
    **/
    IntervalHandlingMethod intervalHandlingMethod() const;

    /**
     * Sets the desired \a method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see intervalHandlingMethod()
    **/
    void setIntervalHandlingMethod( IntervalHandlingMethod method );

    /**
     * Sets the fixed datetime \a range for the temporal properties.
     *
     * \see fixedTemporalRange()
    */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed datetime range for these temporal properties.
     *
     * \see setFixedTemporalRange()
    */
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets the fixed reference datetime \a range. This is to be used for
     * bi-temporal based data. Where data can have both nominal and reference times.
     *
     * \see fixedReferenceTemporalRange()
    */
    void setFixedReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed reference datetime range.
     *
     * \see setFixedReferenceTemporalRange()
    */
    const QgsDateTimeRange &fixedReferenceTemporalRange() const;

    /**
     * Sets the requested temporal \a range to retrieve when
     * returning data from the associated data provider.
     *
     * \note this is not normally manually set, and is intended for use by
     * QgsRasterLayerRenderer to automatically set the requested temporal range
     *  on a clone of the data provider during a render job.
     *
     * \see requestedTemporalRange()
    */
    void setRequestedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the requested temporal range.
     * Intended to be used by the provider in fetching data.
     *
     * \see setRequestedTemporalRange()
    */
    const QgsDateTimeRange &requestedTemporalRange() const;

    /**
     * Sets the requested reference temporal \a range to retrieve when
     * returning data from the associated data provider.
     *
     * \note this is not normally manually set, and is intended for use by
     * QgsRasterLayerRenderer to automatically set the requested temporal range
     *  on a clone of the data provider during a render job.
     *
     * \see requestedReferenceTemporalRange()
    */
    void setRequestedReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the requested reference temporal range.
     * Intended to be used by the provider in fetching data.
     *
     * \see setRequestedReferenceTemporalRange()
    */
    const QgsDateTimeRange &requestedReferenceTemporalRange() const;

    /**
     * Sets the time enabled status.
     * This enables whether time part in the temporal range should be
     * used when updated the temporal range of these capabilities.
     *
     * This is useful in some temporal layers who use dates only.
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

    //! Represents the requested temporal range.
    QgsDateTimeRange mRequestedRange;

    //! Represents the requested reference temporal range.
    QgsDateTimeRange mRequestedReferenceRange;

    /**
     * Stores the fixed reference temporal range
     */
    QgsDateTimeRange mFixedReferenceRange;

    //! If reference range has been enabled to be used in these properties
    bool mReferenceEnable = false;

    //! Interval handling method
    IntervalHandlingMethod mFetchMode = MatchUsingWholeRange;

    //! Temporal capabilities mode.
    TemporalMode mMode = ModeTemporalRangeFromDataProvider;

};

#endif // QGSRASTERDATAPROVIDERTEMPORALCAPABILITIES_H
