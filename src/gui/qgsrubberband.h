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
#ifndef QGSRUBBERBAND_H
#define QGSRUBBERBAND_H

#include "qgsmapcanvasitem.h"
#include "qgsgeometry.h"
#include <QBrush>
#include <QList>
#include <QPen>
#include <QPolygon>

class QgsVectorLayer;
class QPaintEvent;

/** \ingroup gui
 * A class for drawing transient features (e.g. digitising lines) on the map.
 */
class GUI_EXPORT QgsRubberBand: public QgsMapCanvasItem
{
  public:

    /** Icons
     *  Added in 1.9 */
    enum IconType
    {
      ICON_NONE,
      ICON_CROSS,
      ICON_X,
      ICON_BOX,
      ICON_CIRCLE
    };

    /**
     * Creates a new RubberBand.
     * @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     * @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     * Added in 1.9.
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas, QGis::GeometryType geometryType = QGis::Line );
    /**
     * Creates a new RubberBand.
     * @deprecated
     * @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     * @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas, bool isPolygon );
    ~QgsRubberBand();

    /** Set the color for the rubberband */
    void setColor( const QColor & color );

    /** Set the width of the line. Outline width for polygon. */
    void setWidth( int width );

    /** Set the icon type to highlight point geometries.
     *  Added in 1.9 */
    void setIcon( IconType icon );

    /** Set the size of the point icons
      *  Added in 1.9 */
    void setIconSize( int iconSize );

    /**
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to geometryType.
     * @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     * Added in 1.9.
     */
    void reset( QGis::GeometryType geometryType = QGis::Line );
    /**
     * @deprecated
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to isPolygon.
     * @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    void reset( bool isPolygon );

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
     In contrast to addGeometry, this method does also change the geometry type of the rubberband.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    */
    void setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**Sets this rubber band to a map canvas rectangle
      @param rect rectangle in canvas coordinates
      @note added in version 1.7*/
    void setToCanvasRectangle( const QRect& rect );

    /**Add the geometry of an existing feature to a rubberband
     This is useful for multi feature highlighting.
    @param geom the geometry object
    @param layer the layer containing the feature, used for coord transformation to map
    crs. In case of 0 pointer, the coordinates are not going to be transformed.
    @note added in 1.5
    */
    void addGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**Adds translation to original coordinates (all in map coordinates)*/
    void setTranslationOffset( double dx, double dy );

    /**Returns number of geometries
     * added in 1.5 */
    int size() const;

    /**Returns count of vertices in all lists of mPoint*/
    int numberOfVertices() const;

    /**Return vertex*/
    const QgsPoint *getPoint( int i, int j = 0 ) const;

    /**Returns the rubberband as a Geometry.
    * added in 1.6 */
    QgsGeometry* asGeometry();

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

  private:
    QBrush mBrush;
    QPen mPen;

    /** The width of any line within the rubberband. */
    int mWidth;

    /** The size of the icon for points.
      * Added in 1.9 */
    int mIconSize;

    /** Icon to be shown.
     *  Added in 1.9 */
    IconType mIconType ;

    /**Nested lists used for multitypes*/
    QList< QList <QgsPoint> > mPoints;
    QGis::GeometryType mGeometryType;
    double mTranslationOffsetX;
    double mTranslationOffsetY;

    QgsRubberBand();

    static QgsPolyline getPolyline( const QList<QgsPoint> & points );

};

#endif
