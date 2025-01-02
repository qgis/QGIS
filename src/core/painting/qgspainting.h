/***************************************************************************
    qgspainting.h
    ---------------------
    begin                : July 2016
    copyright            : (C) 2016 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPAINTING_H
#define QGSPAINTING_H

#include "qgis_core.h"
#include "qgis.h"
#include "qgis_sip.h"

#include <QPainter>

class QTransform;

/**
 * \ingroup core
 * \brief Contains miscellaneous painting utility functions.
 *
 */
class CORE_EXPORT QgsPainting
{
  public:

    /**
     * Returns a QPainter::CompositionMode corresponding to a Qgis::BlendMode.
     *
     * \see getBlendModeEnum
     */
    static QPainter::CompositionMode getCompositionMode( Qgis::BlendMode blendMode );

    /**
     * Returns a Qgis::BlendMode corresponding to a QPainter::CompositionMode.
     *
     * \see getCompositionMode()
     */
    static Qgis::BlendMode getBlendModeEnum( QPainter::CompositionMode blendMode );

    /**
     * Returns TRUE if \a mode is a clipping blend mode.
     *
     * \since QGIS 3.30
     */
    static bool isClippingMode( Qgis::BlendMode mode );

    /**
     * Calculates the QTransform which maps the triangle defined by the points (\a inX1, \a inY1), (\a inY2, \a inY2), (\a inX3, \a inY3)
     * to the triangle defined by (\a outX1, \a outY1), (\a outY2, \a outY2), (\a outX3, \a outY3).
     *
     * \param inX1 source triangle vertex 1 x-coordinate
     * \param inY1 source triangle vertex 1 y-coordinate
     * \param inX2 source triangle vertex 2 x-coordinate
     * \param inY2 source triangle vertex 2 y-coordinate
     * \param inX3 source triangle vertex 3 x-coordinate
     * \param inY3 source triangle vertex 3 y-coordinate
     * \param outX1 destination triangle vertex 1 x-coordinate
     * \param outY1 destination triangle vertex 1 y-coordinate
     * \param outX2 destination triangle vertex 2 x-coordinate
     * \param outY2 destination triangle vertex 2 y-coordinate
     * \param outX3 destination triangle vertex 3 x-coordinate
     * \param outY3 destination triangle vertex 3 y-coordinate
     * \param ok will be set to TRUE if the transform could be determined.
     *
     * \returns Calculated transform (if possible)
     *
     * \since QGIS 3.34
     */
    static QTransform triangleToTriangleTransform( double inX1, double inY1, double inX2, double inY2, double inX3, double inY3, double outX1, double outY1, double outX2, double outY2, double outX3, double outY3, bool &ok SIP_OUT );

    /**
     * Draws a \a triangle onto a \a painter using a mapped texture image.
     *
     * The triangle will be rendered using the portion of the texture image described by the triangle (\a textureX1, \a textureY1), (\a textureX2, \a textureY2), (\a textureX3, \a textureY3).
     * Texture coordinates should be in the range 0-1 (as a fraction of the image size), where (0, 0) coorresponds to the top-left of the texture image.
     *
     * The caller must ensure that \a triangle is a closed QPolygonF consisting of 4 vertices (the 3 triangle vertices + the first vertex again to close the polygon).
     *
     * Returns TRUE if the triangle could be rendered, or FALSE if it could not (e.g. when the described points are co-linear).
     *
     * \since QGIS 3.34
     */
    static bool drawTriangleUsingTexture(
      QPainter *painter,
      const QPolygonF &triangle,
      const QImage &textureImage,
      float textureX1, float textureY1,
      float textureX2, float textureY2,
      float textureX3, float textureY3
    );

    /**
     * Returns the default Qt horizontal DPI.
     *
     * \note This method proxies the internal Qt qt_defaultDpiX() function.
     *
     * \see qtDefaultDpiY()
     * \since QGIS 3.40
     */
    static int qtDefaultDpiX();

    /**
     * Returns the default Qt vertical DPI.
     *
     * \note This method proxies the internal Qt qt_defaultDpiY() function.
     *
     * \see qtDefaultDpiX()
     * \since QGIS 3.40
     */
    static int qtDefaultDpiY();

    /**
     * Applies a workaround to a \a painter to avoid an issue with incorrect scaling
     * when drawing QPictures.
     *
     * \note This is a low-level method, which alters the \a painter state and relies on the
     * caller saving/restoring painter state accordingly. Consider using
     * the high-level drawPicture() method instead.
     *
     * \see drawPicture()
     * \since QGIS 3.40
     */
    static void applyScaleFixForQPictureDpi( QPainter *painter );

    /**
     * Draws a picture onto a \a painter, correctly applying workarounds to avoid issues
     * with incorrect scaling.
     *
     * \see applyScaleFixForQPictureDpi()
     * \since QGIS 3.40
     */
    static void drawPicture( QPainter *painter, const QPointF &point, const QPicture &picture );
};

#endif // QGSPAINTING_H
