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
 *
 * \warning This is not considered stable API, and may change in future QGIS releases. It is
 * exposed to the Python bindings as a tech preview only.
 *
 * \since QGIS 3.0
 */
class _3D_EXPORT QgsPhongMaterialSettings
{
  public:

    /**
     * Constructor for QgsPhongMaterialSettings.
     */
    QgsPhongMaterialSettings() = default;

    //! Returns ambient color component
    QColor ambient() const { return mAmbient; }
    //! Returns diffuse color component
    QColor diffuse() const { return mDiffuse; }
    //! Returns specular color component
    QColor specular() const { return mSpecular; }
    //! Returns shininess of the surface
    float shininess() const { return mShininess; }

    /**
     * Returns whether the diffuse texture is used.
     *
     * \note Diffuse textures will only be used at render time if diffuseTextureEnabled() is TRUE
     * and a texturePath() is non-empty.
     *
     * \see shouldUseDiffuseTexture()
     * \see setDiffuseTextureEnabled()
     * \see texturePath()
     */
    bool diffuseTextureEnabled() const { return mDiffuseTextureEnabled; }

    /**
     * Returns whether the diffuse texture should be used during rendering.
     *
     * Diffuse textures will only be used at render time if diffuseTextureEnabled() is TRUE
     * and a texturePath() is non-empty.
     *
     * \see diffuseTextureEnabled()
     * \see texturePath()
     */
    bool shouldUseDiffuseTexture() const { return mDiffuseTextureEnabled && !mTexturePath.isEmpty(); }

    /**
     * Returns the diffuse texture path.
     *
     * \note Diffuse textures will only be used at render time if diffuseTextureEnabled() is TRUE
     * and a texturePath() is non-empty.
     *
     * \see setTexturePath()
     * \see diffuseTextureEnabled()
     */
    QString texturePath() const { return mTexturePath; }

    /**
     * Returns the texture scale
     * The texture scale changes the size of the displayed texture in the 3D scene
     * If the texture scale is less than 1 the texture will be stretched
     */
    float textureScale() const { return mTextureScale; }

    //! Returns the texture's rotation in degrees
    float textureRotation() const { return mTextureRotation; }

    //! Sets ambient color component
    void setAmbient( const QColor &ambient ) { mAmbient = ambient; }
    //! Sets diffuse color component
    void setDiffuse( const QColor &diffuse ) { mDiffuse = diffuse; }
    //! Sets specular color component
    void setSpecular( const QColor &specular ) { mSpecular = specular; }
    //! Sets shininess of the surface
    void setShininess( float shininess ) { mShininess = shininess; }

    /**
     * Sets whether the diffuse texture is enabled.
     *
     * \note Diffuse textures will only be used at render time if diffuseTextureEnabled() is TRUE
     * and a texturePath() is non-empty.
     *
     * \see diffuseTextureEnabled()
     * \see setTexturePath()
     */
    void setDiffuseTextureEnabled( bool used ) { mDiffuseTextureEnabled = used; }

    /**
     * Sets the \a path of the texture.
     *
     * \note Diffuse textures will only be used at render time if diffuseTextureEnabled() is TRUE
     * and a texturePath() is non-empty.
     *
     * \see texturePath()
     * \see setDiffuseTextureEnabled()
     */
    void setTexturePath( const QString &path ) { mTexturePath = path; }

    /**
     * Sets the texture scale
     * The texture scale changes the size of the displayed texture in the 3D scene
     * If the texture scale is less than 1 the texture will be stretched
     */
    void setTextureScale( float scale ) { mTextureScale = scale; }

    //! Sets the texture rotation in degrees
    void setTextureRotation( float rotation ) { mTextureRotation = rotation; }

    //! Reads settings from a DOM element
    void readXml( const QDomElement &elem );
    //! Writes settings to a DOM element
    void writeXml( QDomElement &elem ) const;

    bool operator==( const QgsPhongMaterialSettings &other ) const
    {
      return mAmbient == other.mAmbient &&
             mDiffuse == other.mDiffuse &&
             mSpecular == other.mSpecular &&
             mShininess == other.mShininess &&
             mDiffuseTextureEnabled == other.mDiffuseTextureEnabled &&
             mTexturePath == other.mTexturePath &&
             mTextureScale == other.mTextureScale &&
             mTextureRotation == other.mTextureRotation;
    }

  private:
    QColor mAmbient{ QColor::fromRgbF( 0.1f, 0.1f, 0.1f, 1.0f ) };
    QColor mDiffuse{ QColor::fromRgbF( 0.7f, 0.7f, 0.7f, 1.0f ) };
    QColor mSpecular{ QColor::fromRgbF( 1.0f, 1.0f, 1.0f, 1.0f ) };
    float mShininess = 0.0f;
    bool mDiffuseTextureEnabled{ false };
    QString mTexturePath;
    float mTextureScale{ 1.0f };
    float mTextureRotation{ 0.0f };
};


#endif // QGSPHONGMATERIALSETTINGS_H
