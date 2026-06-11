/***************************************************************************
  qgspbrmaterial.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPBRMATERIAL_H
#define QGSPBRMATERIAL_H

#include "qgis.h"
#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QObject>

#define SIP_NO_FILE

namespace Qt3DRender
{

  class QFilterKey;
  class QEffect;
  class QAbstractTexture;
  class QTechnique;
  class QParameter;
  class QShaderProgram;
  class QShaderProgramBuilder;
  class QRenderPass;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief Base class for Physically Based Rendering (PBR) materials.
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsPBRMaterial : public QgsMaterial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsPBRMaterial, with the specified \a parent node.
     */
    explicit QgsPBRMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsPBRMaterial() override;

  public slots:
    //! Must be an SRGB color
    void setBaseColor( const QColor &baseColor );

    /**
     * Sets the base color texture. Takes ownership.
     *
     * \warning Make sure the texture format is correctly set, eg by setting to SRGB format wherever appropriate.
     */
    void setBaseColorTexture( Qt3DRender::QAbstractTexture *baseColor );

    //! Set constant roughness value (between 0 - 1.0)
    void setRoughness( float roughness );

    //! Sets the roughness texture. Takes ownership
    void setRoughnessTexture( Qt3DRender::QAbstractTexture *roughness );

    //! Sets the ambient occlusion texture. Takes ownership. Set to NULLPTR to remove.
    void setAmbientOcclusionTexture( Qt3DRender::QAbstractTexture *ambientOcclusion );

    //! Sets the normal texture map. Takes ownership. Set to NULLPTR to remove.
    void setNormalTexture( Qt3DRender::QAbstractTexture *normal );

    //! Sets the height texture map. Takes ownership. Set to NULLPTR to remove.
    void setHeightTexture( Qt3DRender::QAbstractTexture *height );

    //! Sets the parallax \a scale factor, which impacts the effect the height texture map has on the material.
    void setParallaxScale( double scale );

    /**
     * Enables or disables instanced point rendering mode.
     *
     * When \a enabled is TRUE the material uses the instanced vertex shader.
     * \a flags controls which per-instance attributes (scale, rotation) are active.
     */
    void setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags );

    void setTextureScale( float textureScale );
    void setTextureRotation( float textureRotation );
    void setTextureOffset( float textureOffsetX, float textureOffsetY );

    void setFlatShadingEnabled( bool enabled );

    void setOpacity( float opacity );

    /**
     * Switches between data-defined (per-vertex attribute) and uniform color mode.
     * When \a enabled is TRUE, the pbrDataDefined.vert shader is used and
     * the DATA_DEFINED define is injected into the fragment shader.
     */
    void setDataDefinedEnabled( bool enabled );

    /**
     * When data defined texture translation is enabled,
     * the fragment shader uses per-instance
     * translation, rotation, and scale attributes for the texture coordinates.
     */
    void setDataDefinedTextureTransformEnabled( bool enabled );

    /**
     * Sets whether environmental lighting effects should be enabled for the material.
     *
     * By default this is disabled.
     */
    void setEnvironmentalLightingEnabled( bool enabled );

  protected:
    //! Must be called by subclasses
    void initMaterial();

    virtual QStringList fragmentShaderDefines() const;

    void updateShaders();

    Qt3DRender::QEffect *mEffect = nullptr;

  private:
    Qt3DRender::QParameter *mBaseColorParameter = nullptr;
    Qt3DRender::QParameter *mRoughnessParameter = nullptr;
    Qt3DRender::QParameter *mBaseColorMapParameter = nullptr;
    Qt3DRender::QParameter *mRoughnessMapParameter = nullptr;
    Qt3DRender::QParameter *mAmbientOcclusionMapParameter = nullptr;
    Qt3DRender::QParameter *mNormalMapParameter = nullptr;
    Qt3DRender::QParameter *mHeightMapParameter = nullptr;
    Qt3DRender::QParameter *mParallaxScaleParameter = nullptr;
    Qt3DRender::QParameter *mTextureScaleParameter = nullptr;
    Qt3DRender::QParameter *mTextureRotationParameter = nullptr;
    Qt3DRender::QParameter *mTextureOffsetParameter = nullptr;
    Qt3DRender::QParameter *mOpacityParameter = nullptr;
    Qt3DRender::QTechnique *mMetalRoughGL3Technique = nullptr;
    Qt3DRender::QRenderPass *mMetalRoughGL3RenderPass = nullptr;
    Qt3DRender::QShaderProgram *mMetalRoughGL3Shader = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;

    bool mUsingBaseColorMap = false;
    bool mUsingRoughnessMap = false;
    bool mUsingAmbientOcclusionMap = false;
    bool mUsingNormalMap = false;
    bool mUsingHeightMap = false;
    bool mFlatShading = false;
    bool mDataDefinedEnabled = false;
    bool mInstanced = false;
    Qgis::InstancedMaterialFlags mInstanceFlags;
    bool mDataDefinedTextureTransformEnabled = false;
    bool mEnableEnvironmentalLighting = false;

    friend class TestQgsGltf3DUtils;
};

///@endcond PRIVATE

#endif // QGSPBRMATERIAL_H
