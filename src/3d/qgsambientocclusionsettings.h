/***************************************************************************
  qgsambientocclusionsettings.h
  --------------------------------------
  Date                 : June 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAMBIENTOCCLUSIONSETTINGS_H
#define QGSAMBIENTOCCLUSIONSETTINGS_H

#include <QString>
#include <QMap>

#include "qgis_3d.h"

class QgsReadWriteContext;
class QDomElement;

#define SIP_NO_FILE

/**
 * \brief class containing the configuration of ambient occlusion rendering
 * \ingroup 3d
 * \note Not available in Python bindings
 * \since QGIS 3.28
 */
class _3D_EXPORT QgsAmbientOcclusionSettings
{
  public:
    //! Default constructor
    QgsAmbientOcclusionSettings() = default;
    //! Copy constructor
    QgsAmbientOcclusionSettings( const QgsAmbientOcclusionSettings &other );
    //! delete assignment operator
    QgsAmbientOcclusionSettings &operator=( QgsAmbientOcclusionSettings const &rhs );

    //! Reads settings from a DOM \a element
    void readXml( const QDomElement &element, const QgsReadWriteContext &context );
    //! Writes settings to a DOM \a element
    void writeXml( QDomElement &element, const QgsReadWriteContext &context ) const;

    //! Sets whether ambient occlusion effect is enabled
    void setAmbientOcclusionEnabled( bool enabled ) { mEnabled = enabled; }

    //! Returns whether ambient occlusion effect is enabled
    bool ambientOcclusionEnabled() const { return mEnabled; }

    //! Sets whether the ambient occlusion texture will be blurred
    void setBlurringEnabled( bool enabled ) { mBlurEnabled = enabled; }

    //! Retuens whether the ambient occlusion texture is blurred
    bool blurringEnabled() const { return mBlurEnabled; }

    //! Sets the shading factor of the ambient occlusion effect
    void setShadingFactor( float factor ) { mShadingFactor = factor; }

    //! Returns the shading factor of the ambient occlusion effect
    float shadingFactor() const { return mShadingFactor; }

    //! Sets the distance attenuation factor of the ambient occlusion effect
    void setDistanceAttenuationFactor( float factor ) { mDistanceAttenuationFactor = factor; }

    //! Returns the distance attenuation factor of the ambient occlusion effect
    float distanceAttenuationFactor() const { return mDistanceAttenuationFactor; }

    //! Sets the radius parameter of the ambient occlusion effect
    void setRadiusParameter( float radius ) { mRadiusParameter = radius; }

    //! Returns the radius parameter of the ambient occlusion effect
    float radiusParameter() const { return mRadiusParameter; }

  private:
    bool mEnabled = false;
    bool mBlurEnabled = true;
    float mShadingFactor = 50.0f;
    float mDistanceAttenuationFactor = 500.0f;
    float mRadiusParameter = 0.05f;
};

#endif // QGSAMBIENTOCCLUSIONSETTINGS_H
