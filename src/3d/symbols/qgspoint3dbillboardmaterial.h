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

#include "qgsmarkersymbollayer.h"
#include "qgs3dmapsettings.h"

class QgsPoint3DBillboardMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

  public:
    QgsPoint3DBillboardMaterial();

    void setSize( const QSizeF size );
    QSizeF size() const;

    void setViewportSize( const QSizeF size );
    QSizeF windowSize() const;

    void setTexture2D( Qt3DRender::QTexture2D *texture2D );
    Qt3DRender::QTexture2D *texture2D();

    void setTexture2DFromImagePath( QString imagePath );

    void setTexture2DFromImage( QImage image, double size = 100 );

    void useDefaultSymbol( const Qgs3DMapSettings &map, bool selected = false );

    void setTexture2DFromSymbol( QgsMarkerSymbol *markerSymbol, const Qgs3DMapSettings &map, bool selected = false );

    void setTexture2DFromTextureImage( Qt3DRender::QAbstractTextureImage *textureImage );

  private:
    Qt3DRender::QParameter *mSize = nullptr;
    Qt3DRender::QParameter *mViewportSize = nullptr;
    Qt3DRender::QParameter *mTexture2D = nullptr;
};


#endif // QGSPOINT3DBILLBOARDMATERIAL_H
