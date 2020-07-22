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

class QgsImageTexture : public Qt3DRender::QPaintedTextureImage
{
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
