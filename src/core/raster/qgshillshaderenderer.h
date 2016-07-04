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


#include "qgsrasterrenderer.h"

class QgsRasterBlock;
class QgsRectangle;
class QgsRasterInterface;


/**
 * \ingroup core
 * @brief A renderer for generating live hillshade models.
 * @note added in QGIS 2.16
 */
class CORE_EXPORT QgsHillshadeRenderer : public QgsRasterRenderer
{
  public:
    /**
     * @brief A renderer for generating live hillshade models.
     * @param input The input raster interface
     * @param band The band in the raster to use
     * @param lightAzimuth The azimuth of the light source
     * @param lightAltitude The altitude of the light source
     */
    QgsHillshadeRenderer( QgsRasterInterface* input, int band, double lightAzimuth, double lightAltitude );

    QgsHillshadeRenderer * clone() const override;

    /**
     * @brief Factory method to create a new renderer
     * @param elem A DOM element to create the renderer from.
     * @param input The raster input interface.
     * @return A new QgsHillshadeRenderer.
     */
    static QgsRasterRenderer* create( const QDomElement& elem, QgsRasterInterface* input );

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const override;

    QgsRasterBlock *block( int bandNo, QgsRectangle  const & extent, int width, int height ) override;

    QList<int> usesBands() const override;

    /** Returns the band used by the renderer
     */
    int band() const { return mBand; }

    /** Sets the band used by the renderer.
     * @see band
     */
    void setBand( int bandNo );

    /**
     * Returns the direction of the light over the raster between 0-360.
     * @see setAzimuth()
     */
    double azimuth() const { return mLightAzimuth; }

    /** Returns the angle of the light source over the raster.
     * @see setAltitude()
     */
    double altitude()  const { return mLightAngle; }

    /** Returns the Z scaling factor.
     * @see setZFactor()
     */
    double zFactor()  const { return mZFactor; }

    /** Returns true if the renderer is using multi-directional hillshading.
     * @see setMultiDirectional()
     */
    bool multiDirectional() const { return mMultiDirectional; }

    /**
     * @brief Set the azimuth of the light source.
     * @param azimuth The azimuth of the light source, between 0 and 360.0
     * @see azimuth()
     */
    void setAzimuth( double azimuth ) { mLightAzimuth = azimuth; }

    /**
     * @brief Set the altitude of the light source
     * @param altitude the altitude
     * @see altitude()
     */
    void setAltitude( double altitude ) { mLightAngle = altitude; }

    /**
     * @brief Set the Z scaling factor of the result image.
     * @param zfactor The z factor
     * @see zFactor()
     */
    void setZFactor( double zfactor ) { mZFactor = zfactor; }

    /** Sets whether to render using a multi-directional hillshade algorithm.
     * @param isMultiDirectional set to true to use multi directional rendering
     * @see multiDirectional()
     */
    void setMultiDirectional( bool isMultiDirectional ) { mMultiDirectional = isMultiDirectional; }

  private:
    int mBand;
    double mZFactor;
    double mLightAngle;
    double mLightAzimuth;
    bool mMultiDirectional;

    /** Calculates the first order derivative in x-direction according to Horn (1981)*/
    double calcFirstDerX( double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33 , double cellsize );

    /** Calculates the first order derivative in y-direction according to Horn (1981)*/
    double calcFirstDerY( double x11, double x21, double x31, double x12, double x22, double x32, double x13, double x23, double x33 , double cellsize );
};

#endif // QGSHILLSHADERENDERER_H
