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

class QgsGCPCanvasItem;

class QgsGeorefDataPoint
{
  public:

    //! constructor
    QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas, int id,
                        const QgsPoint& pixelCoords,
                        const QgsPoint& mapCoords );

    ~QgsGeorefDataPoint();


    //! returns coordinates of the point
    QgsPoint pixelCoords() const { return mPixelCoords; }
    QgsPoint mapCoords()   const { return mMapCoords; }

  private:
    QgsGCPCanvasItem *mGCPSourceItem;
    QgsGCPCanvasItem *mGCPDestinationItem;
    int mId;
    QgsPoint mPixelCoords;
    QgsPoint mMapCoords;
};
