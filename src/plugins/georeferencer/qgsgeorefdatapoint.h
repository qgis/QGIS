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

#ifndef QGSGEOREFDATAPOINT_H
#define QGSGEOREFDATAPOINT_H

#include "qgsmapcanvasitem.h"

class QgsGCPCanvasItem;

class QgsGeorefDataPoint : public QObject
{
    Q_OBJECT

  public:
    //! constructor
    QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas,
                        const QgsPoint& pixelCoords, const QgsPoint& mapCoords,
                        bool enable );
    QgsGeorefDataPoint( const QgsGeorefDataPoint &p );
    ~QgsGeorefDataPoint();

    //! returns coordinates of the point
    QgsPoint pixelCoords() const { return mPixelCoords; }
    void setPixelCoords( const QgsPoint &p );

    QgsPoint mapCoords() const { return mMapCoords; }
    void setMapCoords( const QgsPoint &p );

    bool isEnabled() const { return mEnabled; }
    void setEnabled( bool enabled );

    int id() const { return mId; }
    void setId( int id );

    bool contains( QPoint p, bool isMapPlugin );

    QgsMapCanvas *srcCanvas() const { return mSrcCanvas; }
    QgsMapCanvas *dstCanvas() const { return mDstCanvas; }

    QPointF residual() const { return mResidual; }
    void setResidual( QPointF r );

  public slots:
    void moveTo( QPoint, bool isMapPlugin );
    void updateCoords();

  private:
    QgsMapCanvas *mSrcCanvas;
    QgsMapCanvas *mDstCanvas;
    QgsGCPCanvasItem *mGCPSourceItem;
    QgsGCPCanvasItem *mGCPDestinationItem;
    QgsPoint mPixelCoords;
    QgsPoint mMapCoords;

    int mId;
    bool mEnabled;
    QPointF mResidual;
};

#endif //QGSGEOREFDATAPOINT_H
