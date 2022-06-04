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
#include "qgsabstractmaterialsettings.h"
#include "qgspropertycollection.h"

#include <QColor>

#ifndef SIP_RUN
namespace Qt3DRender
{
  class QGeometry;
}
#endif //SIP_RUN

class QDomElement;

/**
 * \ingroup 3d
 * \brief Basic shading material used for rendering based on the Phong shading model
 * with three color components: ambient, diffuse and specular.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPhongMaterialSettings : public QgsAbstractMaterialSettings
{
  public:

    /**
     * Constructor for QgsPhongMaterialSettings.
     */
    QgsPhongMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the Phong material.
     */
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique technique );

    /**
     * Returns a new instance of QgsPhongMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsPhongMaterialSettings *clone() const override SIP_FACTORY;

    //! Returns ambient color component
    QColor ambient() const { return mAmbient; }
    //! Returns diffuse color component
    QColor diffuse() const { return mDiffuse; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    float shininess() const { return mShininess; }

    /**
     * Returns the opacity of the surface
     * \since QGIS 3.26
     */
    float opacity() const { return mOpacity; }

    QMap<QString, QString> toExportParameters() const override;

    //! Sets ambient color component
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }
    //! Sets diffuse color component
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( float shininess ) { mShininess = shininess; }

    /**
     * Sets shininess of the surface
     * \since QGIS 3.26
     */
    void setOpacity( float opacity ) { mOpacity = opacity; }



    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

#ifndef SIP_RUN
    Qt3DRender::QMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect ) const override;

    QByteArray dataDefinedVertexColorsAsByte( const QgsExpressionContext &expressionContext ) const override;
    int dataDefinedByteStride() const override;
    void applyDataDefinedToGeometry( Qt3DRender::QGeometry *geometry, int vertexCount, const QByteArray &data ) const override;
#endif

    // TODO c++20 - replace with = default
    bool operator==( const QgsPhongMaterialSettings &other ) const
    {
      return mAmbient == other.mAmbient &&
             mDiffuse == other.mDiffuse &&
             mSpecular == other.mSpecular &&
             mShininess == other.mShininess;
    }

  private:
    QColor mAmbient{ QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) };
    QColor mDiffuse{ QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) };
    QColor mSpecular{ QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    float mShininess = 0.0f;
    float mOpacity = 1.0f;

    //! Constructs a material from shader files
    Qt3DRender::QMaterial *dataDefinedMaterial() const;
};


#endif // QGSPHONGMATERIALSETTINGS_H
