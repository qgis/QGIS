/***************************************************************************
  qgsgoochmaterialsettings.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGOOCHMATERIALSETTINGS_H
#define QGSGOOCHMATERIALSETTINGS_H

#include "qgis_3d.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup 3d
 * Basic shading material used for rendering based on the Phong shading model
 * with three color components: ambient, diffuse and specular.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsGoochMaterialSettings : public QgsAbstractMaterialSettings
{
  public:

    /**
     * Constructor for QgsGoochMaterialSettings.
     */
    QgsGoochMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns a new instance of QgsGoochMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsGoochMaterialSettings *clone() const override SIP_FACTORY;

    //! Returns warm color component
    QColor warm() const { return mWarm; }

    //! Returns cool color component
    QColor cool() const { return mCool; }

    //! Returns diffuse color component
    QColor diffuse() const { return mDiffuse; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    float shininess() const { return mShininess; }

    //! Sets warm color component
    void setWarm( const QColor &warm ) { mWarm = warm; }

    //! Sets cool color component
    void setCool( const QColor &cool ) { mCool = cool; }

    //! Sets diffuse color component
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( float shininess ) { mShininess = shininess; }

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
#ifndef SIP_RUN
    Qt3DRender::QMaterial *toMaterial( const QgsMaterialContext &context ) const override SIP_FACTORY;
    QgsLineMaterial *toLineMaterial( const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect ) const override;
#endif

    bool operator==( const QgsGoochMaterialSettings &other ) const
    {
      return mDiffuse == other.mDiffuse &&
             mSpecular == other.mSpecular &&
             mWarm == other.mWarm &&
             mCool == other.mCool &&
             mShininess == other.mShininess;
    }

  private:
    QColor mDiffuse{ QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) };
    QColor mSpecular{ QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    QColor mWarm { QColor( 107, 0, 107 )};
    QColor mCool { QColor( 255, 130, 0 )};
    float mShininess = 0.0f;
};


#endif // QGSGOOCHMATERIALSETTINGS_H
