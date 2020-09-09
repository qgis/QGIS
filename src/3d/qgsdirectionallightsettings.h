/***************************************************************************
  qgsdirectionallightsettings.h
  --------------------------------------
  Date                 : June 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDIRECTIONALLIGHTSETTINGS_H
#define QGSDIRECTIONALLIGHTSETTINGS_H
#include "qgis_3d.h"

#include "qgsvector3d.h"
#include <QColor>

class QDomDocument;
class QDomElement;

/**
 * \ingroup 3d
 * Definition of a directional light in a 3D map scene
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsDirectionalLightSettings
{
  public:
    //! Construct a directional light with default values
    QgsDirectionalLightSettings() = default;

    //! Returns the direction of the light in degrees
    QgsVector3D direction() const { return mDirection; }
    //! Sets the direction of the light in degrees
    void setDirection( const QgsVector3D &direction ) { mDirection = direction; }

    //! Returns color of the light
    QColor color() const { return mColor; }
    //! Sets color of the light
    void setColor( const QColor &color ) { mColor = color; }

    //! Returns intensity of the light
    float intensity() const { return mIntensity; }
    //! Sets intensity of the light
    void setIntensity( float intensity ) { mIntensity = intensity; }

    //! Writes configuration to a new DOM element and returns it
    QDomElement writeXml( QDomDocument &doc ) const;
    //! Reads configuration from a DOM element previously written using writeXml()
    void readXml( const QDomElement &elem );

    bool operator==( const QgsDirectionalLightSettings &other );

  private:
    QgsVector3D mDirection = QgsVector3D( 0.0, -1.0, 0.0 );
    QColor mColor = Qt::white;
    float mIntensity = 0.5;
};

#endif // QGSDIRECTIONALLIGHTSETTINGS_H
