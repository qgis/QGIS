/***************************************************************************
  qgsoverlaytextureentity.h
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

#ifndef QGSOVERLAYTEXTUREENTITY_H
#define QGSOVERLAYTEXTUREENTITY_H

#include "qgsrenderpassquad.h"

class QgsFrameGraph;
namespace Qt3DRender
{
  class QTexture2D;
}

#define SIP_NO_FILE

/**
 * \ingroup qgis_3d
 * \brief An entity responsible for rendering an overlay texture in 3D view.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.44
 */
class QgsOverlayTextureEntity : public QgsRenderPassQuad
{
    Q_OBJECT

  public:
    //! Constructor
    QgsOverlayTextureEntity( Qt3DRender::QTexture2D *texture, Qt3DRender::QLayer *layer, QNode *parent = nullptr );

    /**
     * Sets the texture debugging parameters
     * \param corner the texture's corner position within the window
     * \param size the texture's size
     * \param offset the offset between the corner of the window and the texture
     *
     * \since QGIS 4.0
     */
    void setPosition( Qt::Corner corner, QSizeF size, QSizeF offset = QSizeF( 0., 0. ) );

    //! Sets the texture debugging parameters
    void setPosition( Qt::Corner corner, double size, double offset = 0. );

  private:
    //! Sets the view port of the quad
    void setViewport( const QPointF &centerTexCoords, const QSizeF &sizeTexCoords );

  protected:
    Qt3DRender::QParameter *mTextureParameter = nullptr;
    Qt3DRender::QParameter *mCenterTextureCoords = nullptr;
    Qt3DRender::QParameter *mSizeTextureCoords = nullptr;
    Qt3DRender::QParameter *mIsDepth = nullptr;
    Qt3DRender::QParameter *mFlipTextureY = nullptr;
};

#endif // QGSOVERLAYTEXTUREENTITY_H
