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

#include "qgs3dmapsettings.h"

#define SIP_NO_FILE

/**
 * \ingroup 3d
 * \brief Material of the billboard rendering for points in 3D map view.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
class QgsPoint3DBillboardMaterial : public Qt3DRender::QMaterial
{
    Q_OBJECT

  public:
    QgsPoint3DBillboardMaterial();

    //! Set the billboard size.
    void setSize( const QSizeF size );
    //! Returns the billboard size.
    QSizeF size() const;

    //! Set the size of the view port.
    void setViewportSize( const QSizeF size );
    //! Returns the size of the view port.
    QSizeF windowSize() const;

    //! Set default symbol for the texture with \a map and \a selected parameter for rendering.
    void useDefaultSymbol( const Qgs3DMapSettings &map, bool selected = false );

    //! Set \a markerSymbol for the texture with \a map and \a selected parameter for rendering.
    void setTexture2DFromSymbol( QgsMarkerSymbol *markerSymbol, const Qgs3DMapSettings &map, bool selected = false );

  private:
    //! Set the texture2D of the billboard from \a image with \a size.
    void setTexture2DFromImage( QImage image, double size = 100 );

    //! Set texture2D from \a textureImage
    void setTexture2DFromTextureImage( Qt3DRender::QAbstractTextureImage *textureImage );

    Qt3DRender::QParameter *mSize = nullptr;
    Qt3DRender::QParameter *mViewportSize = nullptr;
    Qt3DRender::QParameter *mTexture2D = nullptr;
};


#endif // QGSPOINT3DBILLBOARDMATERIAL_H
