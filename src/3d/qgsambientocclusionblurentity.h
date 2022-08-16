/***************************************************************************
  qgsambientocclusionblurentity.h
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

#ifndef QGSAMBIENTOCCLUSIONBLURENTITY_H
#define QGSAMBIENTOCCLUSIONBLURENTITY_H

#include "qgsrenderpassquad.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief An entity that is responsible for blurring the ambient occlusion factor texture.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.28
 */
class QgsAmbientOcclusionBlurEntity : public QgsRenderPassQuad
{
    Q_OBJECT
  public:
    //! Constructor
    QgsAmbientOcclusionBlurEntity( Qt3DRender::QTexture2D *texture, QNode *parent = nullptr );
  private:
    Qt3DRender::QParameter *mAmbientOcclusionFactorTextureParameter = nullptr;
};

#endif // QGSAMBIENTOCCLUSIONBLURENTITY_H
