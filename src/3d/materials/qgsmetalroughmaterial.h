/***************************************************************************
  qgsmetalroughmaterial.h
  --------------------------------------
  Date                 : December 2023
  Copyright            : (C) 2023 by Nyall Dawson
  Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMETALROUGHMATERIAL_H
#define QGSMETALROUGHMATERIAL_H

#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QObject>

#define SIP_NO_FILE

// adapted from Qt's qmetalroughmaterial.h
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
 * \brief A PBR metal rough material.
 * \since QGIS 3.36
 */
class _3D_EXPORT QgsMetalRoughMaterial : public QgsMaterial
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsMetalRoughMaterial, with the specified \a parent node.
     */
    explicit QgsMetalRoughMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsMetalRoughMaterial() override;

  public slots:
    void setBaseColor( const QVariant &baseColor );
    void setMetalness( const QVariant &metalness );
    void setRoughness( const QVariant &roughness );
    void setAmbientOcclusion( const QVariant &ambientOcclusion );
    void setNormal( const QVariant &normal );
    void setEmission( const QVariant &emission );
    void setEmissionFactor( double factor );
    void setTextureScale( float textureScale );
    void setTextureRotation( float textureRotation );
    void setFlatShadingEnabled( bool enabled );

  private:
    void init();

    void updateFragmentShader();

    Qt3DRender::QParameter *mBaseColorParameter = nullptr;
    Qt3DRender::QParameter *mMetalnessParameter = nullptr;
    Qt3DRender::QParameter *mRoughnessParameter = nullptr;
    Qt3DRender::QParameter *mBaseColorMapParameter = nullptr;
    Qt3DRender::QParameter *mMetalnessMapParameter = nullptr;
    Qt3DRender::QParameter *mRoughnessMapParameter = nullptr;
    Qt3DRender::QParameter *mAmbientOcclusionMapParameter = nullptr;
    Qt3DRender::QParameter *mNormalMapParameter = nullptr;
    Qt3DRender::QParameter *mEmissionMapParameter = nullptr;
    Qt3DRender::QParameter *mEmissionFactorParameter = nullptr;
    Qt3DRender::QParameter *mTextureScaleParameter = nullptr;
    Qt3DRender::QParameter *mTextureRotationParameter = nullptr;
    Qt3DRender::QEffect *mMetalRoughEffect = nullptr;
    Qt3DRender::QTechnique *mMetalRoughGL3Technique = nullptr;
    Qt3DRender::QRenderPass *mMetalRoughGL3RenderPass = nullptr;
    Qt3DRender::QShaderProgram *mMetalRoughGL3Shader = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;
    bool mUsingBaseColorMap = false;
    bool mUsingMetalnessMap = false;
    bool mUsingRoughnessMap = false;
    bool mUsingAmbientOcclusionMap = false;
    bool mUsingNormalMap = false;
    bool mUsingEmissionMap = false;
    bool mFlatShading = false;

    friend class TestQgsGltf3DUtils;
};

///@endcond PRIVATE

#endif // QGSMETALROUGHMATERIAL_H
