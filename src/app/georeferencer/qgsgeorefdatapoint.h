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
#include "qgscoordinatereferencesystem.h"

class QgsGCPCanvasItem;

class QgsGeorefDataPoint : public QObject
{
    Q_OBJECT

  public:
    //! constructor
    QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas,
                        const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationMapCoords,
                        const QgsCoordinateReferenceSystem &destinationCrs, bool enable );
    QgsGeorefDataPoint( const QgsGeorefDataPoint &p );
    ~QgsGeorefDataPoint() override;

    /**
     * Returns source coordinates of the point.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     */
    QgsPointXY sourceCoords() const { return mSourceCoords; }
    void setSourceCoords( const QgsPointXY &p );

    QgsPointXY destinationMapCoords() const { return mDestinationMapCoords; }
    void setDestinationMapCoords( const QgsPointXY &p );

    QgsPointXY transCoords() const;
    void setTransCoords( const QgsPointXY &p );

    /**
     * Returns the destination point in canvas coordinates (i.e. pixels).
     *
     * May be an empty point if not yet calculated.
     */
    QgsPointXY destinationInCanvasPixels() const;

    /**
     * Sets the destination point in canvas coordinates (i.e. pixels).
     */
    void setDestinationInCanvasPixels( const QgsPointXY &p );

    bool isEnabled() const { return mEnabled; }
    void setEnabled( bool enabled );

    int id() const { return mId; }
    void setId( int id );

    bool contains( QPoint p, bool isMapPlugin );

    QgsMapCanvas *srcCanvas() const { return mSrcCanvas; }
    QgsMapCanvas *dstCanvas() const { return mDstCanvas; }

    QPointF residual() const { return mResidual; }
    void setResidual( QPointF r );

    QgsCoordinateReferenceSystem destinationCrs() const { return mDestinationCrs; }

  public slots:
    void moveTo( QPoint canvasPixels, bool isMapPlugin );
    void updateCoords();

  private:
    QgsMapCanvas *mSrcCanvas = nullptr;
    QgsMapCanvas *mDstCanvas = nullptr;
    QgsGCPCanvasItem *mGCPSourceItem = nullptr;
    QgsGCPCanvasItem *mGCPDestinationItem = nullptr;
    QgsPointXY mSourceCoords;
    QgsPointXY mDestinationMapCoords;
    QgsPointXY mTransCoords;

    // destination point converted to canvas coordinates (i.e. pixels)
    QgsPointXY mDestinationInCanvasPixels;

    int mId;
    QgsCoordinateReferenceSystem mDestinationCrs;
    bool mEnabled;
    QPointF mResidual;

    QgsGeorefDataPoint &operator=( const QgsGeorefDataPoint & ) = delete;
};

#endif //QGSGEOREFDATAPOINT_H
