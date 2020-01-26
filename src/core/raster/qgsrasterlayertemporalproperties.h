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
#include "qgsrasterdataprovider.h"

/**
 * \class QgsRasterLayerTemporalProperties
 * \ingroup core
 * Implementation of map layer temporal properties for raster layers.
 *
 * \since QGIS 3.14
 */
class CORE_EXPORT QgsRasterLayerTemporalProperties : public QgsMapLayerTemporalProperties
{
  public:

    /**
     * Constructor for QgsRasterLayerTemporalProperties.
     *
     * The \a enabled argument specifies whether the temporal properties are initially enabled or not (see isActive()).
     */
    QgsRasterLayerTemporalProperties( bool enabled = false );

    virtual ~QgsRasterLayerTemporalProperties() = default;

    /**
     * Mode of the raster temporal properties
     **/
    enum TemporalMode
    {
      ModeFixedTemporalRange, //! Mode when temporal properties have fixed start and end datetimes.
      ModeTemporalRangeFromDataProvider, //! Mode when raster layer depends on temporal range from its data provider.
      ModeTemporalRangesList //! To be used when raster layer has list of temporal ranges.
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
     * \see fixedTemporalRange()
     */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the fixed temporal range for the layer.
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     *\see setFixedTemporalRange()
    **/
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Sets the temporal \a range to apply to the whole layer.
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeProviderTemporalRange
     *
     * \see providerTemporalRange()
     */
    void setProviderTemporalRange( const QgsDateTimeRange &range );

    /**
     * Returns the provider temporal range for these properties
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeProviderTemporalRange
     *
     *\see setProviderTemporalRange()
    **/
    const QgsDateTimeRange &providerTemporalRange() const;

    /**
     * Sets the raster layer properties with WMS-T temporal settings.
     *
     * \param dimension contains text content indicating WMS layer available time value(s).
     *
     * \warning This is to be used to support WMS-T layers only. Applicable when
     * TemporalMode is QgsRasterLayerTemporalProperties::ModeFixedTemporalRange or
     * QgsRasterLayerTemporalProperties::ModeTemporalRangesList
     *
    **/
    void setWmstRelatedSettings( const QString &dimension );

    /**
     * Sets the raster layer properties with WMS-T temporal settings.
     *
     * \param dimension contains text content indicating WMS layer available time value(s).
     */
    void setDataProvider( QgsRasterDataProvider *provider );

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

  private:

    //! Temporal layer mode.
    TemporalMode mMode = TemporalMode::ModeFixedTemporalRange;

    //! Represents datetime range member.
    QgsDateTimeRange mRange;

    //! Raster layer data provider with temporal properties
    QgsRasterDataProvider *mDataProvider = nullptr;

    /**
     * Returns the temporal mode given index
     **/
    TemporalMode indexToMode( int index );
};

#endif // QGSRASTERLAYERTEMPORALPROPERTIES_H
