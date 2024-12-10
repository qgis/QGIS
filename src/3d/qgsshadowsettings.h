/***************************************************************************
  qgsshadowsettings.h
  --------------------------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb uderscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSHADOWSETTINGS_H
#define QGSSHADOWSETTINGS_H

#include <QString>
#include <QMap>

#include "qgis_3d.h"

class QgsReadWriteContext;
class QDomElement;

#define SIP_NO_FILE

/**
 * \brief class containing the configuration of shadows rendering
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsShadowSettings
{
  public:
    QgsShadowSettings() = default;
    QgsShadowSettings( const QgsShadowSettings &other );
    QgsShadowSettings &operator=( QgsShadowSettings const &rhs );

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Returns whether shadow rendering is enabled
    bool renderShadows() const { return mRenderShadows; }
    //! Returns the selected direcctional light used to cast shadows
    int selectedDirectionalLight() const { return mSelectedDirectionalLight; }

    /**
     * Returns the maximum shadow rendering distance accounted for when rendering shadows
     * Objects further away from the camera than the specified distance won't cast shadows
     * This helps with producing a reasonable shadow resolution when looking at a large area or up to the sky
     * \since QGIS 3.16
     */
    double maximumShadowRenderingDistance() const { return mMaximumShadowRenderingDistance; }

    /**
     * Returns the shadow bias used to correct the numerical imprecision of shadows (for the depth test)
     * This helps with reducing the self shadowing artifact
     * \since QGIS 3.16
     */
    double shadowBias() const { return mShadowBias; }

    /**
     * Returns the resolution of the shadow map texture used to generate the shadows
     * \since QGIS 3.16
     */
    int shadowMapResolution() const { return mShadowMapResolution; }

    //! Sets whether shadow rendering is enabled
    void setRenderShadows( bool enabled ) { mRenderShadows = enabled; }
    //! Sets which directional light is used to generate shadows
    void setSelectedDirectionalLight( int selectedLight ) { mSelectedDirectionalLight = selectedLight; }

    /**
     * Sets the maximum shadow rendering distance accounted for when rendering shadows
     * Objects further away from the camera than the specified distance won't cast shadows
     * This helps with producing a reasonable shadow resolution when looking at a large area or up to the sky
     * \since QGIS 3.16
     */
    void setMaximumShadowRenderingDistance( double distance ) { mMaximumShadowRenderingDistance = distance; }

    /**
     * Sets the shadow bias value
     * Tweak this to reduce artifacts like self shadowing
     * A reasonable range of values can be between a very small positive value like 0.00000001 and 0.1
     * \since QGIS 3.16
     */
    void setShadowBias( double shadowBias ) { mShadowBias = shadowBias; }

    /**
     * Sets the resolution of the shadow map texture (this can be used to generate higher quality shadows)
     * \since QGIS 3.16
     */
    void setShadowMapResolution( int resolution ) { mShadowMapResolution = resolution; }

  private:
    bool mRenderShadows = false;
    int mSelectedDirectionalLight = 0;
    double mMaximumShadowRenderingDistance = 1500.0;
    double mShadowBias = 0.00001;
    int mShadowMapResolution = 2048;
};

#endif // QGSSKYBOXSETTINGS_H
