/***************************************************************************
  qgsdepthentity.h
  --------------------------------------
  Date                 : June 2024
  Copyright            : (C) 2024 by Benoit De Mezzo
  Email                : benoit dot de dot mezzo at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDEPTHENTITY_H
#define QGSDEPTHENTITY_H

#include "qgsrenderpassquad.h"

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief An entity that is responsible for capturing depth.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class QgsDepthEntity : public QgsRenderPassQuad
{
    Q_OBJECT

  public:
    //! Constructor
    QgsDepthEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent = nullptr );
};

#endif // QGSDEPTHENTITY_H
