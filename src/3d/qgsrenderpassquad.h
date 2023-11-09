/***************************************************************************
  qgsrenderpassquad.h
  --------------------------------------
  Date                 : June 2022
  Copyright            : (C) 2022 by Belgacem Nedjima
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERPASSQUAD_H
#define QGSRENDERPASSQUAD_H

#include <Qt3DCore/QEntity>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QCamera>
#include <Qt3DRender/QShaderProgram>
#include <Qt3DRender/QLayer>

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief An entity that is responsible for rendering a screen quad for a specific rendering pass.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.28
 */
class QgsRenderPassQuad : public Qt3DCore::QEntity
{
    Q_OBJECT
  public:
    //! Constructor
    QgsRenderPassQuad( QNode *parent = nullptr );

    //! Returns the layer object used to select this entity for rendering in a specific rendering pass
    Qt3DRender::QLayer *layer() { return mLayer; }
  protected:
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QShaderProgram *mShader = nullptr;
    Qt3DRender::QLayer *mLayer = nullptr;
};

#endif // QGSRENDERPASSQUAD_H
