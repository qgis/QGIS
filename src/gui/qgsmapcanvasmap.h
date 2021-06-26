/***************************************************************************
    qgsmapcanvasmap.h  -  draws the map in map canvas
    ----------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASMAP_H
#define QGSMAPCANVASMAP_H

#include "qgsmapcanvasitem.h"

class QgsMapSettings;
class QgsMapCanvas;

#define SIP_NO_FILE

/// @cond PRIVATE

/**
 * \ingroup gui
 * A rectangular graphics item representing the map on the canvas.
 *
 * \note This class is not a part of public API
 */
class QgsMapCanvasMap : public QgsMapCanvasItem
{
  public:

    //! constructor
    QgsMapCanvasMap( QgsMapCanvas *canvas );

    //! \since QGIS 2.4
    void setContent( const QImage &image, const QgsRectangle &rect );

    //! \since QGIS 2.4
    QImage contentImage() const { return mImage; }

    void paint( QPainter *painter ) override;

    void addPreviewImage( const QImage &image, const QgsRectangle &rect );

    QRectF boundingRect() const override;

  private:

    QImage mImage;

    //! Preview images for panning. Usually cover area around the rendered image
    QList< QPair< QImage, QgsRectangle > > mPreviewImages;
};

/// @endcond

#endif
