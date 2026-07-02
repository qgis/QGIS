/***************************************************************************
    qgshighlightmaterial.h
    ---------------------
    begin                : December 2025
    copyright            : (C) 2025 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSHIGHLIGHTMATERIAL_H
#define QGSHIGHLIGHTMATERIAL_H

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
 * \brief A single color material for highlighting features.
 * Uses the highlight color and opacity defined in qgis settings Map/highlight
 * \since QGIS 4.0
 */
class _3D_EXPORT QgsHighlightMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsHighlightMaterial, using the specified \a technique and \a parent node.
     */
    explicit QgsHighlightMaterial( Qt3DCore::QNode *parent = nullptr );
    ~QgsHighlightMaterial() override;

    void setInstancingEnabled( bool enabled, Qgis::InstancedMaterialFlags flags );

    /**
     * Sets the transform from mesh space to object space
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
};

///@endcond PRIVATE

#endif // QGSHIGHLIGHTMATERIAL_H
