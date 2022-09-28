/***************************************************************************
  qgs3dsceneexporter.h
  --------------------------------------
  Date                 : July 2020
  Copyright            : (C) 2020 by Belgacem Nedjima
  Email                : gb underscore nedjima at esi dot dz
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSIMAGETEXTURE_H
#define QGSIMAGETEXTURE_H

#include <QImage>
#include <QPainter>
#include <Qt3DCore/QNode>
#include <Qt3DRender/QPaintedTextureImage>

#include "qgis_3d.h"

#define SIP_NO_FILE

/**
 * \brief Holds an image that can be used as a texture in the 3D view
 *
 * \note Not available in Python bindings
 *
 * \ingroup 3d
 * \since QGIS 3.16
 */
class _3D_EXPORT QgsImageTexture : public Qt3DRender::QPaintedTextureImage
{
    Q_OBJECT

  public:
    //! Constructor
    QgsImageTexture( const QImage &image, Qt3DCore::QNode *parent = nullptr );

    //! paints on the current QImage using painter
    void paint( QPainter *painter ) override
    {
      painter->drawImage( mImage.rect(), mImage, mImage.rect() );
    }

    //! Returns the image
    QImage getImage() const { return mImage; }

  private:

    QImage mImage;

};

#endif // QGSIMAGETEXTURE_H
