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

namespace Qt3DRender
{
  class QMaterial;
  class QLayer;
  class QShaderProgram;
} //namespace Qt3DRender

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
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
    QgsRenderPassQuad( Qt3DRender::QLayer *layer, QNode *parent = nullptr );

  protected:
    Qt3DRender::QMaterial *mMaterial = nullptr;
    Qt3DRender::QShaderProgram *mShader = nullptr;
};

#endif // QGSRENDERPASSQUAD_H
