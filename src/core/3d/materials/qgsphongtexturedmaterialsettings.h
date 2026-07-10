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

#include <optional>

#include "qgis_core.h"
#include "qgsabstractmaterialsettings.h"

#include <QColor>

class QgsMaterial;
class QDomElement;

/**
 * \ingroup core
 * \brief A Phong shading model with diffuse texture map.
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.16
 */
class CORE_EXPORT QgsPhongTexturedMaterialSettings : public QgsAbstractMaterialSettings
{
  public:
    QgsPhongTexturedMaterialSettings() = default;

    QString type() const override;

    /**
     * Returns TRUE if the specified \a technique is supported by the Phong material.
     */
    static bool supportsTechnique( Qgis::MaterialRenderingTechnique technique );

    /**
     * Returns a new instance of QgsPhongTexturedMaterialSettings.
     */
    static QgsAbstractMaterialSettings *create() SIP_FACTORY;

    QgsPhongTexturedMaterialSettings *clone() const override SIP_FACTORY;
    bool equals( const QgsAbstractMaterialSettings *other ) const override;
    QSet< QgsAbstractMaterialSettings::Property > supportedProperties() const override;

    //! Returns ambient color component
    QColor ambient() const { return mAmbient; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    double shininess() const { return mShininess; }

    /**
     * Returns the diffuse texture path.
     *
     * \see setDiffuseTexturePath()
     */
    QString diffuseTexturePath() const { return mDiffuseTexturePath; }

    /**
     * Returns the texture scale.
     *
     * The texture scale changes the size of the material's textures in the 3D scene.
     *
     * If the texture scale is less than 1 the textures will be stretched.
     *
     * \see setTextureScale()
     */
    double textureScale() const { return mTextureScale; }

    /**
     * Returns the texture rotation, in degrees.
     *
     * \see setTextureRotation()
     */
    double textureRotation() const { return mTextureRotation; }

    /**
     * Returns the texture offset.
     *
     * \see setTextureOffset()
     * \since QGIS 4.2
     */
    QPointF textureOffset() const { return mTextureOffset; }

    bool requiresTextureCoordinates() const override;

    /**
     * Returns the opacity of the surface
     * \since QGIS 3.28
     */
    double opacity() const { return mOpacity; }

    //! Sets ambient color component
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }

    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( double shininess ) { mShininess = shininess; }

    /**
     * Sets the \a path of the diffuse texture.
     * \see diffuseTexturePath()
     */
    void setDiffuseTexturePath( const QString &path )
    {
      mDiffuseTexturePath = path;
      mTextureAverageColor.reset();
    }

    /**
     * Sets the texture scale
     * The texture scale changes the size of the displayed texture in the 3D scene
     * If the texture scale is less than 1 the texture will be stretched
     */
    void setTextureScale( double scale ) { mTextureScale = scale; }

    //! Sets the texture rotation in degrees
    void setTextureRotation( double rotation ) { mTextureRotation = rotation; }

    /**
     * Sets the texture \a offset.
     *
     * \see textureOffset()
     * \since QGIS 4.2
     */
    void setTextureOffset( QPointF offset ) { mTextureOffset = offset; }

    /**
     * Sets opacity of the surface.
     * \since QGIS 3.28
     */
    void setOpacity( double opacity ) { mOpacity = opacity; }

    /**
     * Returns an approximate color representing the blended material color.
     *
     * This function calculates a weighted average of the ambient, diffuse, and
     * specular color components to produce a single representative color.
     *
     * \see ambient()
     * \see specular()
     * \see diffuseTexturePath()
     *
     * \since QGIS 4.2
     */
    QColor averageColor() const override;

    /**
     * Decompose an average color into Phong material components, and sets the material's colors accordingly.
     *
     * Sets ambient and specular colors from the input color.
     * This also sets the shininess parameter based on the metallic value.
     *
     * \param baseColor The color to decompose
     * \param metallic Controls how "metal-like" a material appears. Value between 0 and 1
     *
     * \see setAmbient()
     * \see setSpecular()
     * \see setShininess()
     *
     * \since QGIS 4.2
     */
    void setColorsFromBase( const QColor &baseColor, float metallic );

    /**
     * Decomposes a base color into Phong material components.
     *
     * Sets ambient and specular colors from the input color.
     * This is equivalent to calling setColorsFromBase with the metallic parameter equal to 0:
     * setColorsFromBase(baseColor, 0).
     *
     * \param baseColor The color to decompose
     *
     * \see setAmbient()
     * \see setSpecular()
     *
     * \since QGIS 4.2
     */
    void setColorsFromBase( const QColor &baseColor ) override;

    void readXml( const QDomElement &elem, const QgsReadWriteContext &context ) override;
    void writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const override;

    // TODO c++20 - replace with = default
    bool operator==( const QgsPhongTexturedMaterialSettings &other ) const
    {
      return mAmbient == other.mAmbient
             && mSpecular == other.mSpecular
             && mShininess == other.mShininess
             && mOpacity == other.mOpacity
             && mDiffuseTexturePath == other.mDiffuseTexturePath
             && mTextureScale == other.mTextureScale
             && mTextureRotation == other.mTextureRotation
             && qgsDoubleNear( mTextureOffset.x(), other.mTextureOffset.x() )
             && qgsDoubleNear( mTextureOffset.y(), other.mTextureOffset.y() )
             && dataDefinedProperties() == other.dataDefinedProperties();
    }

  private:
    QColor textureAverageColor() const;

  private:
    QColor mAmbient { QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) };
    QColor mSpecular { QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    double mShininess = 0.0;
    double mOpacity = 1.0;
    QString mDiffuseTexturePath;
    double mTextureScale { 1.0f };
    double mTextureRotation { 0.0f };
    QPointF mTextureOffset { 0.0, 0.0 };
    mutable std::optional<QColor> mTextureAverageColor;
};


#endif // QGSPHONGTEXTUREDMATERIALSETTINGS_H
