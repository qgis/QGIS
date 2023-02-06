/***************************************************************************
                         qgsrasterdrawer.h
                         -------------------
    begin                : June 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERDRAWER_H
#define QGSRASTERDRAWER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include <QMap>

class QPainter;
class QImage;
class QgsMapToPixel;
class QgsRenderContext;
struct QgsRasterViewPort;
class QgsRasterBlockFeedback;
class QgsRasterIterator;

/**
 * \ingroup core
 * \brief The drawing pipe for raster layers.
 */
class CORE_EXPORT QgsRasterDrawer
{
  public:

    /**
     * The QgsRasterDrawer constructor.
     * \param iterator the raster iterator to fetch data from
     * \param dpiTarget the target dpi (dots per inch) to be taken into consideration when rendering
     * \deprecated since QGIS 3.28. Use the constructor without the \a dpiTarget argument instead, as DPI is now handled by the draw() method which accepts a QgsRenderContext.
     */
    Q_DECL_DEPRECATED QgsRasterDrawer( QgsRasterIterator *iterator, double dpiTarget ) SIP_DEPRECATED;

    /**
     * The QgsRasterDrawer constructor.
     * \param iterator the raster iterator to fetch data from
     */
    QgsRasterDrawer( QgsRasterIterator *iterator );

    /**
     * Draws raster data.
     * \param p destination QPainter
     * \param viewPort viewport to render
     * \param qgsMapToPixel map to pixel converter
     * \param feedback optional raster feedback object for cancellation/preview. Added in QGIS 3.0.
     */
    void draw( QPainter *p, QgsRasterViewPort *viewPort, const QgsMapToPixel *qgsMapToPixel, QgsRasterBlockFeedback *feedback = nullptr );

    /**
     * Draws raster data.
     * \param context the render context
     * \param viewPort viewport to render
     * \param feedback optional raster feedback object for cancellation/preview.
     * \since QGIS 3.28
     */
    void draw( QgsRenderContext &context, QgsRasterViewPort *viewPort, QgsRasterBlockFeedback *feedback = nullptr );

  protected:

    /**
     * Draws raster part
     * \param p the painter to draw to
     * \param viewPort view port to draw to
     * \param img image to draw
     * \param topLeftCol Left position relative to left border of viewport
     * \param topLeftRow Top position relative to top border of viewport
     * \param mapToPixel map to device coordinate transformation info
     * \note not available in Python bindings
     */
    void drawImage( QPainter *p, QgsRasterViewPort *viewPort, const QImage &img, int topLeftCol, int topLeftRow, const QgsMapToPixel *mapToPixel = nullptr ) const SIP_SKIP;

  private:
    QgsRasterIterator *mIterator = nullptr;
    double mDpiTarget = -1.0;
    double mDpiScaleFactor = 1.0;
};

#endif // QGSRASTERDRAWER_H
