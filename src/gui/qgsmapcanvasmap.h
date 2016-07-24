/***************************************************************************
    qgsmapcanvasmap.h  -  draws the map in map canvas
    ----------------------
    begin                : February 2006
    copyright            : (C) 2006 by Martin Dobias
    email                : wonder.sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPCANVASMAP_H
#define QGSMAPCANVASMAP_H

#include <qgsmapcanvasitem.h>

class QgsMapSettings;
class QgsMapCanvas;

/// @cond PRIVATE

/** \ingroup gui
 * A rectangular graphics item representing the map on the canvas.
 *
 * @note This class is not a part of public API
 */
class QgsMapCanvasMap : public QgsMapCanvasItem
{
  public:

    //! constructor
    QgsMapCanvasMap( QgsMapCanvas* canvas );

    ~QgsMapCanvasMap();

    //! @note added in 2.4
    void setContent( const QImage& image, const QgsRectangle& rect );

    //! @note added in 2.4
    QImage contentImage() const { return mImage; }

    virtual void paint( QPainter * painter ) override;

  private:

    QImage mImage;
};

/// @endcond

#endif
