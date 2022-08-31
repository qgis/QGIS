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
    void setEnabled( bool enabled ) { mEnabled = enabled; }

    //! Returns whether ambient occlusion effect is enabled
    bool isEnabled() const { return mEnabled; }

    //! Sets the shading factor of the ambient occlusion effect
    void setIntensity( float factor ) { mIntensity = factor; }

    //! Returns the shading factor of the ambient occlusion effect
    float intensity() const { return mIntensity; }

    //! Sets the radius parameter of the ambient occlusion effect
    void setRadius( float radius ) { mRadius = radius; }

    //! Returns the radius parameter of the ambient occlusion effect
    float radius() const { return mRadius; }

    //! Sets at what amount of occlusion the effect will kick in
    void setThreshold( float threshold ) { mThreshold = threshold; }

    //! Returns at what amount of occlusion the effect will kick in
    float threshold() const { return mThreshold; }

  private:
    bool mEnabled = false;
    float mIntensity = 0.5f;
    float mRadius = 25.0f;
    float mThreshold = 0.5f;
};

#endif // QGSAMBIENTOCCLUSIONSETTINGS_H
