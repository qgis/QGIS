/***************************************************************************
  qgselevationshadingrenderer.h - QgsElevationShadingRenderer

 ---------------------
 begin                : 4.12.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSELEVATIONSHADINGRENDERER_H
#define QGSELEVATIONSHADINGRENDERER_H

#include "qgis_sip.h"
#include "qgselevationmap.h"
#include "qgis.h"

class QImage;
class QgsElevationMap;
class QgsRenderContext;
class QgsMapSettings;
class QDomDocument;
class QgsReadWriteContext;
class QDomElement;

/**
 * \ingroup core
 * \brief This class can render elevation shading on an image with different methods (eye dome lighting, hillshading, ...).
 *
 * An instance of this class supports the following settings:
 *
 * - activate/deactivate elevation shading
 * - activate/deactivate a particular shading method
 * - parameters of each shading method
 *
 * To render shading on image the caller call renderShading() with the image and
 * an elevation map (see QgsElevationMap()) as parameters. The shading is applied on the image in place.
 * Elevation map and image must have same sizes.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsElevationShadingRenderer
{
  public:
    QgsElevationShadingRenderer();

    /**
     *  Render shading on \a image condidering the elevation map \a elevation and the renderer context \a context
     *  If elevation map and the image don't have same sizes, nothing happens.
     */
    void renderShading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;

    //! Sets whether this shading renderer is active.
    void setActive( bool active );

    //! Returns whether this shading renderer is active.
    bool isActive() const;

    //! Sets active the eye-dome lighting shading method.
    void setActiveEyeDomeLighting( bool active );

    //! Returns whether eye-dome lighting shading method is active
    bool isActiveEyeDomeLighting() const;

    /**
     * Returns the strength of the eye dome lighting method.
     *
     * \see setEyeDomeLightingStrength()
     */
    double eyeDomeLightingStrength() const;

    /**
     * Sets the \a strength of the eye dome lighting method.
     *
     * \see eyeDomeLightingStrength()
     */
    void setEyeDomeLightingStrength( double strength );

    /**
     * Returns the distance of the eye dome lighting method, that is the distance where the
     * effect is apply from the source elevation.
     *
     * \see setEyeDomeLightingDistance()
     */
    double eyeDomeLightingDistance() const;

    /**
     * Sets the distance of the eye dome lighting method, that is the distance where the
     * effect is apply from the source elevation.
     *
     * \see eyeDomeLightingDistance()
     */
    void setEyeDomeLightingDistance( double distance );

    /**
     * Returns the unit of the distance of the eye dome lighting method returned by eyeDomeLightingDistance().
     *
     * \see setEyeDomeLightingDistanceUnit()
     */
    Qgis::RenderUnit eyeDomeLightingDistanceUnit() const;

    /**
     * Sets the \a unit of the distance of the eye dome lighting method set by setEyeDomeLightingDistance().
     *
     * \see eyeDomeLightingDistanceUnit()
     */
    void setEyeDomeLightingDistanceUnit( Qgis::RenderUnit unit );

    //! Sets active the hillshading
    void setActiveHillshading( bool active );

    //! Returns whether the hillshading is active
    bool isActiveHillshading() const;

    /**
     * Returns the z factor used by the hill shading method.
     *
     * \see setHillshadingZFactor()
     */
    double hillshadingZFactor() const;

    /**
     * Sets the z factor used by the hill shading method.
     *
     * \see hillshadingZFactor()
     */
    void setHillshadingZFactor( double zFactor );

    /**
     * Returns whether the hill shading method is multidirectional.
     *
     * \see setHillshadingMultidirectional()
     */
    bool isHillshadingMultidirectional() const;

    /**
     * Sets whether the hill shading method is multidirectional.
     *
     * \see isHillshadingMultidirectional()
     */
    void setHillshadingMultidirectional( bool multiDirectional );

    /**
     * Returns the altitude of the light (degree) that can be used by some methods (e.g. hill shading).
     *
     * \see setLightAltitude()
     */
    double lightAltitude() const;

    /**
     * Sets the altitude of the light (degree) that can be used by some methods (e.g. hill shading).
     *
     * \see setLightAltitude()
     */
    void setLightAltitude( double lightAltitude );

    /**
     * Returns the azimuth of the light (degree) that can be used by some methods (e.g. hill shading).
     *
     * \see setLightAltitude()
     */
    double lightAzimuth() const;

    /**
     * Sets the azimuth of the light (degree) that can be used by some methods (e.g. hill shading).
     *
     * \see lightAzimuth()
     */
    void setLightAzimuth( double lightAzimuth );

    //! Writes configuration on a DOM element
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const;

    //! Reads configuration from a DOM element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns the method used when conbining different elevation sources.
     *
     * \see setCombinedElevationMethod()
     */
    Qgis::ElevationMapCombineMethod combinedElevationMethod() const;

    /**
     * Sets the \a method used when conbining different elevation sources.
     *
     * \see combinedElevationMethod()
     */
    void setCombinedElevationMethod( Qgis::ElevationMapCombineMethod method );

  private:
    bool mIsActive = false;

    Qgis::ElevationMapCombineMethod mCombinedElevationMethod = Qgis::ElevationMapCombineMethod::HighestElevation;

    bool mRenderEdl = true;
    double mEyeDomeLightingStrength = 1000.0;
    double mEyeDomeLightingDistance = 0.5;
    Qgis::RenderUnit mEyeDomeLightingDistanceUnit = Qgis::RenderUnit::Millimeters;

    bool mRenderHillshading = false;
    double mLightAltitude = 45.0;
    double mLightAzimuth = 315.0;
    double mHillshadingZFactor = 1.0;
    bool mHillshadingMultiDir = false;

    void renderEdl( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;
    void renderHillshading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;
};

#endif // QGSELEVATIONSHADINGRENDERER_H
