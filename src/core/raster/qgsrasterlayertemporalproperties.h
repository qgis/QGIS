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
     */
    QgsRasterLayerTemporalProperties( bool enabled = false );

    virtual ~QgsRasterLayerTemporalProperties() = default;

    enum TemporalMode
    {
      ModeFixedTemporalRange,
      ModeTemporalRangeFromDataProvider,
      ModeTemporalRangesList
    };

    /**
     * Return the temporal properties mode
     *
     *\see setMode()
    **/
    TemporalMode mode() const;

    /**
     * Set the temporal properties mode
     *
     *\see mode()
    **/
    void setMode( TemporalMode mode );

    /**
     * Sets the temporal \a range to apply to the whole layer. All bands from
     * the raster layer will be rendered whenever the current datetime range of
     * a render context intersects the specified \a range.
     *
     * For the case of WMS-T layers, new layers with the current datetime range
     * of the render context will be fetched.
     *
     * \warning This setting is only effective when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     * \see fixedTemporalRange()
     */
    void setFixedTemporalRange( const QgsDateTimeRange &range );

    /**
     * Return fixed temporal range for these properties
     *
     * \warning To be used only when mode() is
     * QgsRasterLayerTemporalProperties::ModeFixedTemporalRange
     *
     *\see setFixedTemporalRange()
    **/
    const QgsDateTimeRange &fixedTemporalRange() const;

    /**
     * Set this raster layer properties with WMS-T temporal settings.
     *
     * \warning This is to be used to support WMS-T layers only. Applicable when
     * TemporalMode is QgsRasterLayerTemporalProperties::ModeFixedTemporalRange or
     * QgsRasterLayerTemporalProperties::ModeTemporalRangesList
     *
    **/
    void setWmstRelatedSettings( const QString &dimension );

    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;

    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;

  private:

    //! Temporal layer mode.
    TemporalMode mMode = TemporalMode::ModeFixedTemporalRange;

    //! Represents datetime range member.
    QgsDateTimeRange mRange;

    /**
     * Returns Temporal mode given index
     *
     *
     *
     */
    TemporalMode indexToMode( int index );
};

#endif // QGSRASTERLAYERTEMPORALPROPERTIES_H
