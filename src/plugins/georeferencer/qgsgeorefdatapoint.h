/***************************************************************************
     qgsgeorefdatapoint.h
     --------------------------------------
    Date                 : Sun Sep 16 12:02:56 AKDT 2007
    Copyright            : (C) 2007 by Gary E. Sherman
    Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmapcanvasitem.h"


class QgsGeorefDataPoint : public QgsMapCanvasItem
{
  public:

    //! constructor
    QgsGeorefDataPoint( QgsMapCanvas* mapCanvas, int id,
                        const QgsPoint& pixelCoords,
                        const QgsPoint& mapCoords );

    //! draws point information
    virtual void paint( QPainter* p );

    //! handler for manual updating of position and size
    virtual QRectF boundingRect() const;

    virtual void updatePosition();

    //! returns coordinates of the point
    QgsPoint pixelCoords() { return mPixelCoords; }
    QgsPoint mapCoords() { return mMapCoords; }

  private:
    int mId;
    QgsPoint mPixelCoords;
    QgsPoint mMapCoords;
    QSizeF mTextBounds;
};

