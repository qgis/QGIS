/***************************************************************************
  qgstexturematerial.h
  --------------------------------------
  Date                 : March 2024
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

#ifndef QGSTEXTUREMATERIAL_H
#define QGSTEXTUREMATERIAL_H

#include "qgis.h"
#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QObject>

#define SIP_NO_FILE

class QMatrix4x4;


// adapted from Qt's qtexturematerial.h
namespace Qt3DRender
{

  class QFilterKey;
  class QAbstractTexture;
  class QTechnique;
  class QParameter;
  class QShaderProgram;
  class QRenderPass;

} // namespace Qt3DRender

///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A unlit texture material
 * \since QGIS 3.40
 */
class _3D_EXPORT QgsTextureMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsTextureMaterial, with the specified \a parent node.
     */
    explicit QgsTextureMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsTextureMaterial() override;

    Qt3DRender::QAbstractTexture *texture() const;

    /**
     * Enables or disables instanced point rendering mode.
     * When \a enabled is TRUE the material uses the instanced vertex shader.
     * \a flags controls which per-instance attributes (scale, rotation) are active.
     */
    void setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags );

    /**
     * Sets the transform from mesh space to object space
     * \note Only applies when instancing is enabled
     */
    void setInstancingMeshTransform( const QMatrix4x4 &transform );

  public slots:

    /**
     * Sets the diffuse component of the material.
     * Ownership is transferred to the material.
     *
     * Must be explicitly set to SRGB format.
     */
    void setTexture( Qt3DRender::QAbstractTexture *texture );

  private:
    void init();

    Qt3DRender::QParameter *mTextureParameter = nullptr;
    Qt3DRender::QTechnique *mGL3Technique = nullptr;
    Qt3DRender::QRenderPass *mGL3RenderPass = nullptr;
    Qt3DRender::QShaderProgram *mGL3Shader = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;
    bool mInstanced = false;
    Qgis::InstancedMaterialFlags mInstanceFlags;
    Qt3DRender::QParameter *mTransformParameter = nullptr;
};

///@endcond PRIVATE

#endif // QGSTEXTUREMATERIAL_H
