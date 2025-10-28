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

#include "qgis_3d.h"
#include "qgsmaterial.h"

#define SIP_NO_FILE

class QgsMarkerSymbol;
class Qgs3DRenderContext;

/**
 * \ingroup qgis_3d
 * \brief Material of the billboard rendering for points in 3D map view.
 *
 * This material is designed for use with the QgsBillboardGeometry class providing the billboard geometry.
 *
 * \note Not available in Python bindings
 *
 * \since QGIS 3.10
 */
class _3D_EXPORT QgsPoint3DBillboardMaterial : public QgsMaterial
{
    Q_OBJECT

  public:
    /**
     * Material modes.
     *
     * \since QGIS 4.0
     */
    enum class Mode
    {
      SingleTexture,                //!< Use a single repeated texture for all billboards. Billboard positions should be set using QgsBillboardGeometry::setPositions().
      AtlasTexture,                 //!< Use a texture atlas, so each billboard has a different texture. Billboard positions and texture data should be set using QgsBillboardGeometry::setBillboardData().
      AtlasTextureWithPixelOffsets, //!< Use a texture atlas, so each billboard has a different texture. Billboards have pixel-sized offsets from their position. Billboard positions and texture data should be set using QgsBillboardGeometry::setBillboardData().
    };

    /**
     * Constructor for QgsPoint3DBillboardMaterial, using the specified \a mode.
     */
    QgsPoint3DBillboardMaterial( Mode mode = Mode::SingleTexture );
    ~QgsPoint3DBillboardMaterial() override;

    //! Set the billboard size.
    void setSize( const QSizeF size );
    //! Returns the billboard size.
    QSizeF size() const;

    //! Set the size of the view port.
    void setViewportSize( const QSizeF size );
    //! Returns the size of the view port.
    QSizeF windowSize() const;

    //! Set default symbol for the texture with \a context and \a selected parameter for rendering.
    void useDefaultSymbol( const Qgs3DRenderContext &context, bool selected = false );

    /**
     * Renders a marker symbol to an image.
     *
     * \since QGIS 4.0
     */
    static QImage renderSymbolToImage( const QgsMarkerSymbol *markerSymbol, const Qgs3DRenderContext &context, bool selected = false );

    //! Set \a markerSymbol for the texture with \a context and \a selected parameter for rendering.
    void setTexture2DFromSymbol( const QgsMarkerSymbol *markerSymbol, const Qgs3DRenderContext &context, bool selected = false );

    //! Set the texture2D of the billboard from an \a image.
    void setTexture2DFromImage( const QImage &image );

  private:
    //! Set texture2D from \a textureImage
    void setTexture2DFromTextureImage( Qt3DRender::QAbstractTextureImage *textureImage );

    Qt3DRender::QParameter *mSize = nullptr;
    Qt3DRender::QParameter *mViewportSize = nullptr;
    Qt3DRender::QParameter *mTexture2D = nullptr;
};


#endif // QGSPOINT3DBILLBOARDMATERIAL_H
