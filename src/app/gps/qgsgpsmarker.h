/***************************************************************************
    qgsgpsmarker.h  - canvas item which shows a gps marker
    ---------------------
    begin                : 18 December 2009
    copyright            : (C) 2009 Tim Sutton
    email                : tim at linfiniti com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#ifndef QGSGPSMARKER_H
#define QGSGPSMARKER_H

#include "qgsmapcanvasitem.h"
#include "qgscoordinatereferencesystem.h"
#include "qgspoint.h"

class QPainter;

/** \ingroup app
 * A class for marking the position of a gps pointer.
 */
class QgsGpsMarker : public QgsMapCanvasItem
{
  public:

    QgsGpsMarker( QgsMapCanvas* mapCanvas );

    void setCenter( const QgsPoint& point );

    void paint( QPainter* p );

    QRectF boundingRect() const;

    virtual void updatePosition();

    void setSize( int theSize );

  protected:

    //! coordinates of the point in the center
    QgsPoint mCenter;
    //! Size of the marker - e.g. 8 will draw it as 8x8
    int mSize;

  private:
    QgsCoordinateReferenceSystem mWgs84CRS;

};

#endif
