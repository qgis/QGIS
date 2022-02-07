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

#include "qgis_app.h"
#include "qgsmapcanvasitem.h"
#include "qgscoordinatereferencesystem.h"

class QgsGCPCanvasItem;

/**
 * Contains properties of a ground control point (GCP).
 */
class APP_EXPORT QgsGcpPoint
{
  public:

    /**
     * Constructor for QgsGcpPoint.
     *
     * \param sourceCoordinates source coordinates. This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     * \param destinationPoint destination coordinates
     * \param destinationPointCrs CRS of destination point
     * \param enabled whether the point is currently enabled
     */
    QgsGcpPoint( const QgsPointXY &sourcePoint, const QgsPointXY &destinationPoint,
                 const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled );

    /**
     * Returns the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see setSourcePoint()
     */
    QgsPointXY sourcePoint() const { return mSourcePoint; }

    /**
     * Sets the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see sourcePoint()
     */
    void setSourcePoint( QgsPointXY point ) { mSourcePoint = point; }

    /**
     * Returns the destination coordinates.
     *
     * \see setDestinationPoint()
     */
    QgsPointXY destinationPoint() const { return mDestinationPoint; }

    /**
     * Sets the destination coordinates.
     *
     * \see destinationPoint()
     */
    void setDestinationPoint( QgsPointXY point ) { mDestinationPoint = point; }

    /**
     * Returns the CRS of the destination point.
     *
     * \see setDestinationCrs()
     */
    QgsCoordinateReferenceSystem destinationPointCrs() const;

    /**
     * Sets the \a crs of the destination point.
     *
     * \see destinationCrs()
     */
    void setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns TRUE if the point is currently enabled.
     *
     * \see setEnabled()
     */
    bool isEnabled() const { return mEnabled; }

    /**
     * Sets whether the point is currently enabled.
     *
     * \see enabled()
     */
    void setEnabled( bool enabled ) { mEnabled = enabled; }

  private:

    QgsPointXY mSourcePoint;
    QgsPointXY mDestinationPoint;
    QgsCoordinateReferenceSystem mDestinationCrs;
    bool mEnabled = true;

};


/**
 * Container for a GCP point and the graphical objects which represent it on the map canvas.
 */
class QgsGeorefDataPoint : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeorefDataPoint
     * \param srcCanvas
     * \param dstCanvas
     * \param sourceCoordinates source coordinates. This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     * \param destinationPoint destination coordinates
     * \param destinationPointCrs CRS of destination point
     * \param enabled whether the point is currently enabled
     */
    QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas,
                        const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationPoint,
                        const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled );
    QgsGeorefDataPoint( const QgsGeorefDataPoint &p );
    ~QgsGeorefDataPoint() override;

    /**
     * Returns the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see setSourcePoint()
     */
    QgsPointXY sourcePoint() const { return mGcpPoint.sourcePoint(); }

    /**
     * Sets the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see sourcePoint()
     */
    void setSourcePoint( const QgsPointXY &p );

    /**
     * Returns the destination coordinates.
     *
     * \see setDestinationPoint()
     */
    QgsPointXY destinationPoint() const { return mGcpPoint.destinationPoint(); }

    /**
     * Sets the destination coordinates.
     *
     * \see destinationPoint()
     */
    void setDestinationPoint( const QgsPointXY &p );

    QgsPointXY transCoords() const;
    void setTransCoords( const QgsPointXY &p );

    /**
     * Returns TRUE if the point is currently enabled.
     *
     * \see setEnabled()
     */
    bool isEnabled() const { return mGcpPoint.isEnabled(); }

    /**
     * Sets whether the point is currently enabled.
     *
     * \see enabled()
     */
    void setEnabled( bool enabled );

    int id() const { return mId; }
    void setId( int id );

    bool contains( QPoint p, bool isMapPlugin );

    QgsMapCanvas *srcCanvas() const { return mSrcCanvas; }
    QgsMapCanvas *dstCanvas() const { return mDstCanvas; }

    QPointF residual() const { return mResidual; }
    void setResidual( QPointF r );

    /**
     * Returns the CRS of the destination point.
     *
     * \see setDestinationCrs()
     */
    QgsCoordinateReferenceSystem destinationPointCrs() const { return mGcpPoint.destinationPointCrs(); }

  public slots:
    void moveTo( QPoint canvasPixels, bool isMapPlugin );
    void updateCoords();

  private:
    QgsMapCanvas *mSrcCanvas = nullptr;
    QgsMapCanvas *mDstCanvas = nullptr;
    QgsGCPCanvasItem *mGCPSourceItem = nullptr;
    QgsGCPCanvasItem *mGCPDestinationItem = nullptr;

    QgsGcpPoint mGcpPoint;

    QgsPointXY mTransCoords;

    int mId;
    QPointF mResidual;

    QgsGeorefDataPoint &operator=( const QgsGeorefDataPoint & ) = delete;
};

#endif //QGSGEOREFDATAPOINT_H
