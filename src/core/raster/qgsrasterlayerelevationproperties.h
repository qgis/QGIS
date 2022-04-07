/***************************************************************************
                         qgsrasterlayerelevationproperties.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSRASTERLAYERELEVATIONPROPERTIES_H
#define QGSRASTERLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerelevationproperties.h"

class QgsLineSymbol;

/**
 * \class QgsRasterLayerElevationProperties
 * \ingroup core
 * \brief Raster layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsRasterLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterLayerElevationProperties, with the specified \a parent object.
     */
    QgsRasterLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );
    ~QgsRasterLayerElevationProperties() override;

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QgsRasterLayerElevationProperties *clone() const override SIP_FACTORY;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;

    /**
     * Returns TRUE if the elevation properties are enabled, i.e. the raster layer values represent an elevation surface.
     *
     * \see setEnabled()
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets whether the elevation properties are enabled, i.e. the raster layer values represent an elevation surface.
     *
     * \see isEnabled()
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    /**
     * Returns the band number from which the elevation should be taken.
     *
     * \see setBandNumber()
     */
    int bandNumber() const { return mBandNumber; }

    /**
     * Sets the \a band number from which the elevation should be taken.
     *
     * \see bandNumber()
     */
    void setBandNumber( int band ) { mBandNumber = band; }

    /**
     * Returns the line symbol used to render the raster profile in elevation profile plots.
     *
     * \see setProfileLineSymbol()
     */
    QgsLineSymbol *profileLineSymbol() const;

    /**
     * Sets the line \a symbol used to render the raster profile in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see profileLineSymbol()
     */
    void setProfileLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

  private:

    void setDefaultProfileLineSymbol();

    bool mEnabled = false;
    std::unique_ptr< QgsLineSymbol > mProfileLineSymbol;
    int mBandNumber = 1;

};

#endif // QGSRASTERLAYERELEVATIONPROPERTIES_H
