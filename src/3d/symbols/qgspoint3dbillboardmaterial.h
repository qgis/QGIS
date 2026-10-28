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

#include "qgis.h"
#include "qgis_3d.h"
#include "qgsmaterial.h"

#include <QObject>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

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
     * Additional material attributes.
     *
     * \since QGIS 4.4
     */
    enum class ExtraAttribute
    {
      Size = 1 << 1,
      TextureData = 1 << 2,
      PixelOffsets = 1 << 3,
      VerticalOffset = 1 << 4,
    };
    Q_DECLARE_FLAGS( ExtraAttributes, ExtraAttribute )
    Q_FLAG( ExtraAttributes )

    /**
     * Constructor for QgsPoint3DBillboardMaterial, using the specified additional \a attributes.
     */
    QgsPoint3DBillboardMaterial( ExtraAttributes attributes = ExtraAttributes(), Qgis::BillboardScaleMode scaleMode = Qgis::BillboardScaleMode::ViewIndependent );
    ~QgsPoint3DBillboardMaterial() override;

    //! Set the billboard size.
    void setSize( const QSizeF size );
    //! Returns the billboard size.
    QSizeF size() const;

    /**
     * Set the vertical \a offset.
     *
     * For example, a vertical offset of 0.5 will anchor the billboard's bottom edge at the vertex z position,
     * or -0.5 will anchor the billboard's top edge at the vertex z position.
     *
     * \note This only applies if the material was constructed with the ExtraAttribute::VerticalOffset flag set.
     */
    void setVerticalOffset( float offset );

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
    Qt3DRender::QParameter *mVerticalOffset = nullptr;

    Qgis::BillboardScaleMode mScaleMode = Qgis::BillboardScaleMode::ViewIndependent;
};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsPoint3DBillboardMaterial::ExtraAttributes )


#endif // QGSPOINT3DBILLBOARDMATERIAL_H
