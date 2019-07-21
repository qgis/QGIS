/***************************************************************************
  qgspoint3dbillboardmaterial.h
  --------------------------------------
  Date                 : Jul 2019
  Copyright            : (C) 2019 by Ismail Sunni
  Email                : imajimatika at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPOINT3DBILLBOARDMATERIAL_H
#define QGSPOINT3DBILLBOARDMATERIAL_H

#include <QObject>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QPaintedTextureImage>

class QgsPoint3DBillboardMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

  public:
    QgsPoint3DBillboardMaterial();

    void setSize( const QSizeF size );
    QSizeF size() const;

    void setWindowSize( const QSizeF size );
    QSizeF windowSize() const;

    void setTexture2D( Qt3DRender::QTexture2D *texture2D );
    Qt3DRender::QTexture2D *texture2D();

    void setTexture2DFromImagePath( QString imagePath );

    void setTexture2DFromImage( QImage image );

  private:
    Qt3DRender::QParameter *mSize = nullptr;
    Qt3DRender::QParameter *mWindowSize = nullptr;
    Qt3DRender::QParameter *mTexture2D = nullptr;
};


class QgsBillboardTextureImage : public Qt3DRender::QPaintedTextureImage
{
  public:
    void paint( QPainter *painter );
    void setImage( QImage *image );
    QImage *image();

  private:
    QImage *mImage = nullptr;
};


#endif // QGSPOINT3DBILLBOARDMATERIAL_H
