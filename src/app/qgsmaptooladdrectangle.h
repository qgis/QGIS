/***************************************************************************
    qgsmaptooladdrectangle.h  -  map tool for adding rectangle
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017
    email                : lbartoletti at tuxfamily dot org
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLADDRECTANGLE_H
#define QGSMAPTOOLADDRECTANGLE_H

#include "qgspolygon.h"
#include "qgsmaptoolcapture.h"
#include "qgsbox3d.h"
#include "qgis_app.h"

class QgsPolygon;

class APP_EXPORT QgsMapToolAddRectangle: public QgsMapToolCapture
{
    Q_OBJECT

  public:
    QgsMapToolAddRectangle( QgsMapToolCapture *parentTool, QgsMapCanvas *canvas, CaptureMode mode = CaptureLine );
    ~QgsMapToolAddRectangle() override;

    void keyPressEvent( QKeyEvent *e ) override;
    void keyReleaseEvent( QKeyEvent *e ) override;

    void deactivate( bool isOriented = false );

    void activate() override;
    void clean() override;

  protected:
    explicit QgsMapToolAddRectangle( QgsMapCanvas *canvas ) = delete; //forbidden

    /**
     * The parent map tool, e.g. the add feature tool.
     *  Completed regular shape will be added to this tool by calling its addCurve() method.
     **/
    QgsMapToolCapture *mParentTool = nullptr;
    //! Regular Shape points (in map coordinates)
    QgsPointSequence mPoints;
    //! The rubberband to show the rectangle currently working on
    QgsGeometryRubberBand *mTempRubberBand = nullptr;
    //! Rectangle
    QgsBox3d mRectangle;

    //! Convenient method to export a QgsRectangle to a LineString
    QgsLineString *rectangleToLinestring( bool isOriented = false ) const;
    //! Convenient method to export a QgsRectangle to a Polygon
    QgsPolygon *rectangleToPolygon( bool isOriented = false ) const;

    //! Sets the azimuth. \see mAzimuth
    void setAzimuth( double azimuth );
    //! Sets the first distance. \see mDistance1
    void setDistance1( double distance1 );
    //! Sets the second distance. \see mDistance2
    void setDistance2( double distance2 );
    //! Sets the side. \see mSide
    void setSide( int side );

    //! Returns the azimuth. \see mAzimuth
    double azimuth( ) const { return mAzimuth; }
    //! Returns the first distance. \see mDistance1
    double distance1( ) const { return mDistance1; }
    //! Returns the second distance. \see mDistance2
    double distance2( ) const { return mDistance2; }
    //! Returns the side. \see mSide
    int side( ) const { return mSide; }

    //! Layer type which will be used for rubberband
    QgsWkbTypes::GeometryType mLayerType = QgsWkbTypes::LineGeometry;

  private:
    //! Convenient member for the azimuth of the rotated rectangle or when map is rotated.
    double mAzimuth = 0.0;
    //! Convenient member for the first distance of the rotated rectangle or when map is rotated.
    double mDistance1 = 0.0;
    //! Convenient member for the second distance of the rotated rectangle or when map is rotated.
    double mDistance2 = 0.0;
    //! Convenient member for the side where the second distance is drawn or when map is rotated.
    int mSide = 1;
};

#endif // QGSMAPTOOLADDRECTANGLE_H
