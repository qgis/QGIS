/***************************************************************************
                         qgsrasterlayertemporalproperties.h
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


#ifndef QGSRASTERLAYERTEMPORALPROPERTIES_H
#define QGSRASTERLAYERTEMPORALPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrange.h"
#include "qgsmaplayertemporalproperties.h"
#include "qgsrasterdataprovidertemporalcapabilities.h"

/**
 * \class QgsRasterLayerTemporalProperties
 * \ingroup core
 * Implementation of map layer temporal properties for raster layers.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRasterLayerTemporalProperties : public QgsMapLayerTemporalProperties
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterLayerTemporalProperties, with the specified \a parent object.
     *
     * The \a enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsRasterLayerTemporalProperties( QObject *parent SIP_TRANSFERTHIS = nullptr, bool enabled = false );

    /**
     * Mode of the raster temporal properties
     **/
    enum TemporalMode
    {
      ModeFixedTemporalRange = 0, //!< Mode when temporal properties have fixed start and end datetimes.
      ModeTemporalRangeFromDataProvider = 1, //!< Mode when raster layer depends on temporal range from its dataprovider.
    };

    /**
     * Returns the temporal properties mode.
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Sets the temporal properties \a mode.
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Returns the desired method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see setIntervalHandlingMethod()
    **/
    QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod intervalHandlingMethod() const;

    /**
     * Sets the desired \a method to use when resolving a temporal interval to matching
     * layers or bands in the data provider.
     *
     *\see intervalHandlingMethod()
    **/
    void setIntervalHandlingMethod( QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod method );

    /**
     * Sets a temporal \a range to apply to the whole layer. All bands from
     * the raster layer will be rendered whenever the current datetime range of
     * a render context intersects the specified \a range.
     *
     * For the case of WMS-T layers, this set up will cause new WMS layer to be fetched
     * with which the range of the render context intersects the specified \a range.
     *
     * \warning This setting is only effective when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \note This setting is not set by user. Provider can set this, if it is coming from there.
     *
     * \see fixedTemporalRange()
     */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed temporal range for the layer.
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \see setFixedTemporalRange()
    **/
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets a fixed reference temporal \a range to apply to the whole layer. All bands from
     * the raster layer will be rendered whenever the current datetime range of
     * a render context intersects the specified \a range.
     *
     * For the case of WMS-T layers, this set up will cause new WMS layer to be fetched
     * with which the range of the render context intersects the specified \a range.
     *
     * \warning This setting is only effective when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \note This setting is not set by user. Provider can set this, if it is coming from there.
     *
     * \see fixedReferenceTemporalRange()
     */
    void setFixedReferenceTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed reference temporal range for the layer.
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \see setFixedReferenceTemporalRange()
    **/
    const QgsDateTimeRange &fixedReferenceTemporalRange() const;

    /**
     * Sets the current active datetime range for the temporal properties.
     *
     * \note This can be set by user, through raster layer properties widget.
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
     * This will be used by bi-temporal data.
     *
     * \note This can be set by user, through raster layer properties widget.
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

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

    /**
     * Sets the layers temporal settings to appropriate defaults based on
     * a provider's temporal \a capabilities.
     */
    void setDefaultsFromDataProviderTemporalCapabilities( QgsRasterDataProviderTemporalCapabilities *capabilities );

  private:

    //! Temporal layer mode.
    TemporalMode mMode = ModeFixedTemporalRange;

    //! Temporal layer data fetch mode.
    QgsRasterDataProviderTemporalCapabilities::IntervalHandlingMethod mIntervalHandlingMethod = QgsRasterDataProviderTemporalCapabilities::MatchUsingWholeRange;

    //! Represents fixed temporal range.
    QgsDateTimeRange mFixedRange;

    //! Represents fixed reference temporal range member.
    QgsDateTimeRange mFixedReferenceRange;

    /**
     * Stores reference temporal range
     */
    QgsDateTimeRange mReferenceRange;

    //! Represents current active datetime range member.
    QgsDateTimeRange mRange;

};

#endif // QGSRASTERLAYERTEMPORALPROPERTIES_H
