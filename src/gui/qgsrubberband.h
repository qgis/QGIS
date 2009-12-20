/***************************************************************************
    qgsrubberband.h - Rubberband widget for drawing multilines and polygons
     --------------------------------------
    Date                 : 07-Jan-2006
    Copyright            : (C) 2006 by Tom Elwertowski
    Email                : telwertowski at users dot sourceforge dot net
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */
#ifndef QGSRUBBERBAND_H
#define QGSRUBBERBAND_H

#include "qgsmapcanvasitem.h"
#include <QBrush>
#include <QList>
#include <QPen>
#include <QPolygon>

class QgsGeometry;
class QgsVectorLayer;
class QPaintEvent;

/** \ingroup gui
 * A class for drawing transient features (e.g. digitising lines) on the map.
 */
class GUI_EXPORT QgsRubberBand: public QgsMapCanvasItem
{
  public:
    QgsRubberBand( QgsMapCanvas* mapCanvas, bool isPolygon = false );
    ~QgsRubberBand();

    void setColor( const QColor & color );
    void setWidth( int width );

    void reset( bool isPolygon = false );

    //! Add point to rubberband and update canvas
    //! If adding more points consider using update=false for better performance
    //! geometryIndex is the index of the feature part (in case of multipart geometries)
    void addPoint( const QgsPoint & p, bool update = true, int geometryIndex = 0 );

    //!Removes the last point. Most useful in connection with undo operations
    void removeLastPoint( int geometryIndex = 0 );

    void movePoint( const QgsPoint & p, int geometryIndex = 0 );
    /**Moves the rubber band point specified by index. Note that if the rubber band is
     not used to track the last mouse position, the first point of the rubber band has two vertices*/
    void movePoint( int index, const QgsPoint& p, int geometryIndex = 0 );

    /**Sets this rubber band to the geometry of an existing feature.
     This is useful for feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    @param render the maprender object (used for coord transformation)*/
    void setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**Adds translation to original coordinates (all in map coordinates)*/
    void setTranslationOffset( double dx, double dy );

    /**Returns count of vertices in all lists of mPoint*/
    int numberOfVertices() const;

    /**Return vertex*/
    const QgsPoint *getPoint( int i, int j = 0 ) const;

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

  private:
    QBrush mBrush;
    QPen mPen;

    /**Nested lists used for multitypes*/
    QList< QList <QgsPoint> > mPoints;
    bool mIsPolygon;
    double mTranslationOffsetX;
    double mTranslationOffsetY;

    QgsRubberBand();
};

#endif
