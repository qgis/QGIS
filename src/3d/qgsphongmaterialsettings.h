/***************************************************************************
  qgsphongmaterialsettings.h
  --------------------------------------
  Date                 : July 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGMATERIALSETTINGS_H
#define QGSPHONGMATERIALSETTINGS_H

#include "qgis_3d.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup 3d
 * Basic shading material used for rendering based on the Phong shading model
 * with three color components: ambient, diffuse and specular.
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPhongMaterialSettings
{
  public:
    QgsPhongMaterialSettings()
      : mAmbient( QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) )
      , mDiffuse( QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) )
      , mSpecular( QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) )
    {
    }

    //! Returns ambient color component
    QColor ambient() const { return mAmbient; }
    //! Returns diffuse color component
    QColor diffuse() const { return mDiffuse; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    float shininess() const { return mShininess; }

    //! Sets ambient color component
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }
    //! Sets diffuse color component
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( float shininess ) { mShininess = shininess; }

    //! Reads settings from a DOM element
    void readXml( const QDomElement &elem );
    //! Writes settings to a DOM element
    void writeXml( QDomElement &elem ) const;

  private:
    QColor mAmbient;
    QColor mDiffuse;
    QColor mSpecular;
    float mShininess = 0.0f;
};


#endif // QGSPHONGMATERIALSETTINGS_H
