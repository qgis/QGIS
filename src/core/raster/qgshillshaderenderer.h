/***************************************************************************
                         qgshillshaderenderer.cpp
                         ---------------------------------
    begin                : May 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHILLSHADERENDERER_H
#define QGSHILLSHADERENDERER_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterrenderer.h"

class QgsRasterBlock;
class QgsRectangle;
class QgsRasterInterface;


/**
 * \ingroup core
 * \brief A renderer for generating live hillshade models.
 */
class CORE_EXPORT QgsHillshadeRenderer : public QgsRasterRenderer
{
  public:

    /**
     * \brief A renderer for generating live hillshade models.
     * \param input The input raster interface
     * \param band The band in the raster to use
     * \param lightAzimuth The azimuth of the light source
     * \param lightAltitude The altitude of the light source
     */
    QgsHillshadeRenderer( QgsRasterInterface *input, int band, double lightAzimuth, double lightAltitude );

    QgsHillshadeRenderer *clone() const override SIP_FACTORY;
    Qgis::RasterRendererFlags flags() const override;

    /**
     * \brief Factory method to create a new renderer
     * \param elem A DOM element to create the renderer from.
     * \param input The raster input interface.
     * \returns A new QgsHillshadeRenderer.
     */
    static QgsRasterRenderer *create( const QDomElement &elem, QgsRasterInterface *input ) SIP_FACTORY;

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    QList<int> usesBands() const override;
    int inputBand() const override;

    void toSld( QDomDocument &doc, QDomElement &element, const QVariantMap &props = QVariantMap() ) const override;

    /**
     * Returns the band used by the renderer
     *
     * \deprecated since QGIS 3.38 use inputBand() instead
     */
    Q_DECL_DEPRECATED int band() const SIP_DEPRECATED { return mBand; }

    /**
     * Sets the band used by the renderer.
     * \see band
     *
     * \deprecated since QGIS 3.38 use setInputBand() instead
     */
    Q_DECL_DEPRECATED void setBand( int bandNo ) SIP_DEPRECATED;
    bool setInputBand( int band ) override;

    /**
     * Returns the direction of the light over the raster between 0-360.
     * \see setAzimuth()
     */
    double azimuth() const { return mLightAzimuth; }

    /**
     * Returns the angle of the light source over the raster.
     * \see setAltitude()
     */
    double altitude()  const { return mLightAngle; }

    /**
     * Returns the Z scaling factor.
     * \see setZFactor()
     */
    double zFactor()  const { return mZFactor; }

    /**
     * Returns TRUE if the renderer is using multi-directional hillshading.
     * \see setMultiDirectional()
     */
    bool multiDirectional() const { return mMultiDirectional; }

    /**
     * \brief Set the azimuth of the light source.
     * \param azimuth The azimuth of the light source, between 0 and 360.0
     * \see azimuth()
     */
    void setAzimuth( double azimuth ) { mLightAzimuth = azimuth; }

    /**
     * \brief Set the altitude of the light source
     * \param altitude the altitude
     * \see altitude()
     */
    void setAltitude( double altitude ) { mLightAngle = altitude; }

    /**
     * \brief Set the Z scaling factor of the result image.
     * \param zfactor The z factor
     * \see zFactor()
     */
    void setZFactor( double zfactor ) { mZFactor = zfactor; }

    /**
     * Sets whether to render using a multi-directional hillshade algorithm.
     * \param isMultiDirectional set to TRUE to use multi directional rendering
     * \see multiDirectional()
     */
    void setMultiDirectional( bool isMultiDirectional ) { mMultiDirectional = isMultiDirectional; }

  private:
    int mBand = 1;
    double mZFactor = 1;
    double mLightAngle = 45;
    double mLightAzimuth = 315;
    bool mMultiDirectional = false;

};

#endif // QGSHILLSHADERENDERER_H
