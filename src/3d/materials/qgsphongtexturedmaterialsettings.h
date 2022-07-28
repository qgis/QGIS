/***************************************************************************
  qgsphongtexturedmaterialsettings.h
  --------------------------------------
  Date                 : August 2020
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

#ifndef QGSPHONGTEXTUREDMATERIALSETTINGS_H
#define QGSPHONGTEXTUREDMATERIALSETTINGS_H

#include "qgis_3d.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QDomElement;

/**
 * \ingroup 3d
 * \brief A phong shading model with diffuse texture map.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsPhongTexturedMaterialSettings : public QgsAbstractMaterialSettings
{
  public:

    /**
     * Constructor for QgsPhongTexturedMaterialSettings.
     */
    QgsPhongTexturedMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the Phong material.
     */
    static bool supportsTechnique( QgsMaterialSettingsRenderingTechnique technique );

    /**
     * Returns a new instance of QgsPhongTexturedMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsPhongTexturedMaterialSettings *clone() const override SIP_FACTORY;

    //! Returns ambient color component
    QColor ambient() const { return mAmbient; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    float shininess() const { return mShininess; }

    QMap<QString, QString> toExportParameters() const override;

    /**
     * Returns the diffuse texture path.
     *
     * \see setDiffuseTexturePath()
     */
    QString diffuseTexturePath() const { return mDiffuseTexturePath; }

    /**
     * Returns the texture scale
     * The texture scale changes the size of the displayed texture in the 3D scene
     * If the texture scale is less than 1 the texture will be stretched
     */
    float textureScale() const { return mTextureScale; }

    /**
     * Returns TRUE if the material requires texture coordinates to be generated
     * during triangulation.quiresTextureCoordinates
     */
    bool requiresTextureCoordinates() const { return !mDiffuseTexturePath.isEmpty(); }

    /**
     * Returns the texture rotation, in degrees.
     */
    float textureRotation() const;

    /**
     * Returns the opacity of the surface
     * \since QGIS 3.28
     */
    float opacity() const { return mOpacity; }

    //! Sets ambient color component
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }

    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( float shininess ) { mShininess = shininess; }

    /**
     * Sets the \a path of the diffuse texture.
     * \see diffuseTexturePath()
     */
    void setDiffuseTexturePath( const QString &path ) { mDiffuseTexturePath = path; }

    /**
     * Sets the texture scale
     * The texture scale changes the size of the displayed texture in the 3D scene
     * If the texture scale is less than 1 the texture will be stretched
     */
    void setTextureScale( float scale ) { mTextureScale = scale; }

    //! Sets the texture rotation in degrees
    void setTextureRotation( float rotation ) { mTextureRotation = rotation; }

    /**
     * Sets opacity of the surface.
     * \since QGIS 3.28
     */
    void setOpacity( float opacity ) { mOpacity = opacity; }

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;
#ifndef SIP_RUN
    Qt3DRender::QMaterial *toMaterial( QgsMaterialSettingsRenderingTechnique technique, const QgsMaterialContext &context ) const override SIP_FACTORY;
    void addParametersToEffect( Qt3DRender::QEffect *effect ) const override;
#endif

    // TODO c++20 - replace with = default
    bool operator==( const QgsPhongTexturedMaterialSettings &other ) const
    {
      return mAmbient == other.mAmbient &&
             mSpecular == other.mSpecular &&
             mShininess == other.mShininess &&
             mOpacity == other.mOpacity &&
             mDiffuseTexturePath == other.mDiffuseTexturePath &&
             mTextureScale == other.mTextureScale &&
             mTextureRotation == other.mTextureRotation;
    }

  private:
    QColor mAmbient{ QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) };
    QColor mSpecular{ QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    float mShininess = 0.0f;
    float mOpacity = 1.0f;
    QString mDiffuseTexturePath;
    float mTextureScale{ 1.0f };
    float mTextureRotation{ 0.0f };
};


#endif // QGSPHONGTEXTUREDMATERIALSETTINGS_H
