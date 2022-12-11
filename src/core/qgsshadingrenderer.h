/***************************************************************************
  qgsshadingrenderer.h - QgsShadingRenderer

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
#ifndef QGSSHADINGRENDERER_H
#define QGSSHADINGRENDERER_H

#include "qdom.h"
#include "qgsunittypes.h"
#include "qgis_sip.h"

class QImage;
class QgsElevationMap;
class QgsRenderContext;
class QgsMapSettings;
class QDomDocument;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \brief This class can render shading on image with different methods (eye dome Lighting, hill shading,...).
 * Instane od this class support differents settings:
 * - activate/deactivate the hafind renderer
 * - activate/deactivate a shading methods
 * - parameters of each shading methods
 *
 * To render shading on image the caller call renderShading() with the image and
 * an elevation map (see QgsElevationMap()) as parameters. The shading is applied on the image in place.
 * Elevation map and image must have same sizes.
 *
 * \since QGIS 3.30
 */
class CORE_EXPORT QgsShadingRenderer
{
  public:
    QgsShadingRenderer();

    /**
     *  Render shading on \a image condidering the elevation map \a elevation and the renderer context \a context
     *  If elevation map and the image don't have same sizes, nothing happens.
     */
    void renderShading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;

    //! Sets whether this shading renderer is active.
    void setActive( bool active );

    //! Returns whether this shading renderer is active.
    bool isActive() const;

    //! Sets active the eye dome lighting shading method.
    void setActiveEyeDomeLighting( bool active );

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
    const QgsUnitTypes::RenderUnit &eyeDomeLightingDistanceUnit() const;

    /**
     * Sets the unit of the distance of the eye dome lighting method set by setEyeDomeLightingDistance().
     *
     * \see eyeDomeLightingDistanceUnit()
     */
    void setEyeDomeLightingDistanceUnit( const QgsUnitTypes::RenderUnit &newEyeDomeLightingDistanceUnit );

    //! Sets active the hill shading
    void setActiveHillShading( bool active );

    /**
     * Returns the z factor used by the hill shading method.
     *
     * \see setHillShadingZFactor(à)
     */
    double hillShadingZFactor() const;

    /**
     * Sets the z factor used by the hill shading method.
     *
     * \see hillShadingZFactor()
     */
    void setHillShadingZFactor( double zFactor );

    /**
     * Returns whether the hill shading method is multidirectional.
     *
     * \see setHillShadingMultidirectional()
     */
    bool isHillShadingMultidirectional() const;

    /**
     * Sets whether the hill shading method is multidirectional.
     *
     * \see isHillShadingMultidirectional()
     */
    void setHillShadingMultidirectional( bool multiDirectional );

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
    void writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const;

    //! Reads configuration from a DOM element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );

  private:
    bool mIsActive = true;

    bool mRenderEdl = true;
    double mEyeDomeLightingStrength = 1000.0;
    double mEyeDomeLightingDistance = 0.5;
    QgsUnitTypes::RenderUnit mEyeDomeLightingDistanceUnit = QgsUnitTypes::RenderMillimeters;

    bool mRenderHillShading = false;
    double mLightAltitude = 45.0;
    double mLightAzimuth = 315.0;
    double mHillShadingZFactor = 1.0;
    bool mHillShadingMultiDir = false;

    void renderEdl( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;
    void renderHillShading( const QgsElevationMap &elevation, QImage &image, const QgsRenderContext &context ) const;
};

#endif // QGSSHADINGRENDERER_H
