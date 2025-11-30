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
#include "qgscoordinatereferencesystem.h"
#include "qgsgcppoint.h"
#include "qgsmapcanvasitem.h"

class QgsGCPCanvasItem;
class QgsCoordinateTransformContext;

/**
 * Container for a GCP point and the graphical objects which represent it on the map canvas.
 */
class APP_EXPORT QgsGeorefDataPoint : public QObject
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
    QgsGeorefDataPoint( QgsMapCanvas *srcCanvas, QgsMapCanvas *dstCanvas, const QgsPointXY &sourceCoordinates, const QgsPointXY &destinationPoint, const QgsCoordinateReferenceSystem &destinationPointCrs, bool enabled );
    QgsGeorefDataPoint( const QgsGeorefDataPoint &p );
    ~QgsGeorefDataPoint() override;

    /**
     * Returns the source coordinates.
     *
     * This may either be in pixels (for completely non-referenced images) OR in the source layer CRS.
     *
     * \see setSourcePoint()
     */
    [[nodiscard]] QgsPointXY sourcePoint() const { return mGcpPoint.sourcePoint(); }

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
    [[nodiscard]] QgsPointXY destinationPoint() const { return mGcpPoint.destinationPoint(); }

    /**
     * Sets the destination coordinates.
     *
     * \see destinationPoint()
     */
    void setDestinationPoint( const QgsPointXY &p );

    /**
     * Sets the \a crs of the destination point.
     *
     * \see destinationCrs()
     */
    void setDestinationPointCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns the destinationPoint() transformed to the given target CRS.
     */
    [[nodiscard]] QgsPointXY transformedDestinationPoint( const QgsCoordinateReferenceSystem &targetCrs, const QgsCoordinateTransformContext &context ) const;

    /**
     * Returns TRUE if the point is currently enabled.
     *
     * \see setEnabled()
     */
    [[nodiscard]] bool isEnabled() const { return mGcpPoint.isEnabled(); }

    /**
     * Sets whether the point is currently enabled.
     *
     * \see enabled()
     */
    void setEnabled( bool enabled );

    [[nodiscard]] bool isHovered() const { return mHovered; }

    void setHovered( bool hovered );

    [[nodiscard]] int id() const { return mId; }
    void setId( int id );

    bool contains( const QgsPointXY &p, QgsGcpPoint::PointType type, double &distance );

    [[nodiscard]] QgsMapCanvas *srcCanvas() const { return mSrcCanvas; }
    [[nodiscard]] QgsMapCanvas *dstCanvas() const { return mDstCanvas; }

    [[nodiscard]] QPointF residual() const { return mResidual; }
    void setResidual( QPointF r );

    /**
     * Returns the CRS of the destination point.
     *
     * \see setDestinationCrs()
     */
    [[nodiscard]] QgsCoordinateReferenceSystem destinationPointCrs() const { return mGcpPoint.destinationPointCrs(); }

    [[nodiscard]] QgsGcpPoint point() const { return mGcpPoint; }

  public slots:
    void moveTo( QgsPointXY p, QgsGcpPoint::PointType type );
    void updateCoords();

  private:
    QgsMapCanvas *mSrcCanvas = nullptr;
    QgsMapCanvas *mDstCanvas = nullptr;
    QgsGCPCanvasItem *mGCPSourceItem = nullptr;
    QgsGCPCanvasItem *mGCPDestinationItem = nullptr;
    bool mHovered = false;

    QgsGcpPoint mGcpPoint;

    int mId = -1;
    QPointF mResidual;

    QgsGeorefDataPoint &operator=( const QgsGeorefDataPoint & ) = delete;
};

#endif //QGSGEOREFDATAPOINT_H
