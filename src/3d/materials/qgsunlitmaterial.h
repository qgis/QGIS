/***************************************************************************
    qgsunlitmaterial.h
    ---------------------
    begin                : June 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSUNLITMATERIAL_H
#define QGSUNLITMATERIAL_H

#include "qgis.h"
#include "qgis_3d.h"
#include "qgsmaterial.h"

#define SIP_NO_FILE

class QMatrix4x4;


namespace Qt3DRender
{
  class QParameter;
  class QShaderProgram;
} // namespace Qt3DRender


///@cond PRIVATE

/**
 * \ingroup qgis_3d
 * \brief A single color, unlit material.
 *
 * Unlit materials are not affected by scene lighting.
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsUnlitMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsUnlitMaterial, with the specified \a parent node.
     */
    explicit QgsUnlitMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsUnlitMaterial() override;

    /**
     * Sets the material color.
     *
     * The \a color parameter must specify an sRGB color value. Alpha is supported.
     */
    void setColor( const QColor &color );

    /**
     * Sets whether instancing support is \a enabled for the material.
     */
    void setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags );

    /**
     * Sets the transform from mesh space to object space.
     *
     * \note Only applies when instancing is enabled
     */
    void setInstancingMeshTransform( const QMatrix4x4 &transform );

  private:
    void init();
    void updateShaders();

    Qt3DRender::QShaderProgram *mShaderProgram = nullptr;
    Qgis::MaterialRenderingTechnique mRenderingTechnique;
    bool mInstanced = false;
    Qgis::InstancedMaterialFlags mInstanceFlags;
    Qt3DRender::QParameter *mTransformParameter = nullptr;
    Qt3DRender::QParameter *mNormalTransformParameter = nullptr;
    Qt3DRender::QParameter *mColorParameter = nullptr;
};

///@endcond PRIVATE

#endif // QGSUNLITMATERIAL_H
