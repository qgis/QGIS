/***************************************************************************
  qgsgradientbackgroundentity.h
  --------------------------------------
  Date                 : April 2026
  Copyright            : (C) 2026 by Dominik Cindrić
  Email                : viper dot miniq at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGRADIENTBACKGROUNDENTITY_H
#define QGSGRADIENTBACKGROUNDENTITY_H

#include "qgis_3d.h"

#include <QColor>
#include <Qt3DCore/QEntity>

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QEffect;
  class QFilterKey;
  class QGeometryRenderer;
  class QMaterial;
  class QParameter;
  class QRenderPass;
  class QShaderProgram;
  class QTechnique;
} // namespace Qt3DRender

/**
 * \brief A background entity that renders a two-color gradient behind all 3D scene geometry.
 *
 * The gradient is always oriented top-to-bottom on the screen, independent of camera orientation.
 *
 * \ingroup qgis_3d
 * \since QGIS 4.2
 */
class _3D_EXPORT QgsGradientBackgroundEntity : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    /**
     * Constructs a gradient background entity.
     * \param topColor Color at the top of the gradient.
     * \param bottomColor Color at the bottom of the gradient.
     * \param parent Parent node.
     */
    QgsGradientBackgroundEntity( const QColor &topColor, const QColor &bottomColor, Qt3DCore::QNode *parent = nullptr );

  private:
    Qt3DRender::QEffect *mEffect = nullptr;
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QTechnique *mGl3Technique = nullptr;
    Qt3DRender::QFilterKey *mFilterKey = nullptr;
    Qt3DRender::QRenderPass *mGl3RenderPass = nullptr;
    Qt3DRender::QGeometryRenderer *mMesh = nullptr;
    Qt3DRender::QShaderProgram *mGlShader = nullptr;
    Qt3DRender::QParameter *mTopColorParameter = nullptr;
    Qt3DRender::QParameter *mBottomColorParameter = nullptr;
};

#endif // QGSGRADIENTBACKGROUNDENTITY_H
