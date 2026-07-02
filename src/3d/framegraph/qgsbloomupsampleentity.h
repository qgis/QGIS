/***************************************************************************
  qgsbloomupsampleentity.h
  --------------------------------------
  Date                 : May 2026
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

#ifndef QGSBLOOMUPSAMPLEENTITY_H
#define QGSBLOOMUPSAMPLEENTITY_H

#include "qgsrenderpassquad.h"

#define SIP_NO_FILE

namespace Qt3DRender
{
  class QParameter;
  class QTexture2D;
} //namespace Qt3DRender


/**
 * \ingroup qgis_3d
 * \brief An entity responsible for applying a 9-tap tent upsample filter for physically based bloom.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 4.2
 */
class QgsBloomUpsampleEntity : public QgsRenderPassQuad
{
    Q_OBJECT
  public:
    //! Constructor
    QgsBloomUpsampleEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent = nullptr );

    /**
     * Adds parameters to the entity.
     */
    void addParameters( const QVector<Qt3DRender::QParameter *> &parameters );

  private:
    Qt3DRender::QParameter *mSourceTextureParameter = nullptr;
};

#endif // QGSBLOOMUPSAMPLEENTITY_H
