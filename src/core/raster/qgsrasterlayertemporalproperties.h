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
#include "qgis.h"
#include "qgsrange.h"
#include "qgsmaplayertemporalproperties.h"

/**
 * \class QgsRasterLayerTemporalProperties
 * \ingroup core
 * \brief Implementation of map layer temporal properties for raster layers.
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

    bool isVisibleInTemporalRange( const QgsDateTimeRange &range ) const override;
    QgsDateTimeRange calculateTemporalExtent( QgsMapLayer *layer ) const override SIP_SKIP;
    QList< QgsDateTimeRange > allTemporalRanges( QgsMapLayer *layer ) const override;

    /**
     * Returns the temporal properties mode.
     *
     *\see setMode()
    */
    Qgis::RasterTemporalMode mode() const;

    /**
     * Sets the temporal properties \a mode.
     *
     *\see mode()
    */
    void setMode( Qgis::RasterTemporalMode mode );

    /**
     * Returns flags associated to the temporal property.
     */
    QgsTemporalProperty::Flags flags() const override;

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
     * Sets a temporal \a range to apply to the whole layer. All bands from
     * the raster layer will be rendered whenever the current datetime range of
     * a render context intersects the specified \a range.
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
     * \see setFixedTemporalRange()
    */
    const QgsDateTimeRange &fixedTemporalRange() const;

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

    void setDefaultsFromDataProviderTemporalCapabilities( const QgsDataProviderTemporalCapabilities *capabilities ) override;

  private:

    //! Temporal layer mode.
    Qgis::RasterTemporalMode mMode = Qgis::RasterTemporalMode::FixedTemporalRange;

    //! Temporal layer data fetch mode.
    Qgis::TemporalIntervalMatchMethod mIntervalHandlingMethod = Qgis::TemporalIntervalMatchMethod::MatchUsingWholeRange;

    //! Represents fixed temporal range.
    QgsDateTimeRange mFixedRange;
};

#endif // QGSRASTERLAYERTEMPORALPROPERTIES_H
