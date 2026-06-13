/***************************************************************************
  qgsphongtexturedmaterial.h
  --------------------------------------
  Date                 : August 2024
  Copyright            : (C) 2024 by Jean Felder
  Email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPHONGTEXTUREDMATERIAL_H
#define QGSPHONGTEXTUREDMATERIAL_H

#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QColor>
#include <QObject>
#include <Qt3DRender/QTexture>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QParameter;
  class QShaderProgram;
  class QRenderPass;
  class QFilterKey;
} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A diffuseSpecular material adapted from Qt's qdiffusespecularmaterial.h
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsPhongTexturedMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsPhongTexturedMaterial, with the specified \a parent node.
     */
    explicit QgsPhongTexturedMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsPhongTexturedMaterial() override;

  public slots:
    //! Sets ambient color, must be a SRGB color
    void setAmbient( const QColor &ambient );

    /**
     * Sets the diffuse component of the material.
     * Ownership is transferred to the material.
     *
     * Must be explicitly set to SRGB format.
     */
    void setDiffuseTexture( Qt3DRender::QAbstractTexture *texture );

    void setDiffuseTextureScale( float textureScale );
    void setDiffuseTextureRotation( float textureRotation );
    void setDiffuseTextureOffset( float textureOffsetX, float textureOffsetY );
    //! Sets specular color, must be a SRGB color
    void setSpecular( const QColor &specular );
    void setShininess( float shininess );
    void setOpacity( float opacity );

    /**
     * When data defined texture translation is enabled,
     * the fragment shader uses per-instance
     * translation, rotation, and scale attributes for the texture coordinates.
     */
    void setDataDefinedTextureTransformEnabled( bool enabled );

  private:
    void init();

    void updateVertexShader();

    Qt3DRender::QParameter *mAmbientParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureScaleParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureRotationParameter = nullptr;
    Qt3DRender::QParameter *mDiffuseTextureOffsetParameter = nullptr;
    Qt3DRender::QParameter *mSpecularParameter = nullptr;
    Qt3DRender::QParameter *mShininessParameter = nullptr;
    Qt3DRender::QParameter *mOpacityParameter = nullptr;

    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QTechnique *mGL3Technique = nullptr;
    Qt3DRender::QRenderPass *mGL3RenderPass = nullptr;
    Qt3DRender::QShaderProgram *mGL3Shader = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;

    bool mDataDefinedTextureTransformEnabled = false;
};

///@endcond PRIVATE

#endif // QGSPHONGTEXTUREDMATERIAL_H
