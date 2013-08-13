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
 * It may be used
 */
class GUI_EXPORT QgsRubberBand: public QgsMapCanvasItem
{
  public:

    /** Icons
     *  Added in 1.9 */
    enum IconType
    {
      /**
       * No icon is used
       */
      ICON_NONE,
      /**
       * A cross is used to highlight points (+)
       */
      ICON_CROSS,
      /**
       * A cross is used to highlight points (x)
       */
      ICON_X,
      /**
       * A box is used to highlight points (□)
       */
      ICON_BOX,
      /**
       * A circle is used to highlight points (○)
       */
      ICON_CIRCLE
    };

    /**
     * Creates a new RubberBand.
     *  @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     *  @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     *  @note Added in 1.9.
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas, QGis::GeometryType geometryType = QGis::Line );
    /**
     * Creates a new RubberBand.
     *  @deprecated Use the constructor which takes QGis::GeometryType as second argument instead
     *  @param mapCanvas The map canvas to draw onto. It's CRS will be used map points onto screen coordinates.
     *  @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    QgsRubberBand( QgsMapCanvas* mapCanvas, bool isPolygon );
    ~QgsRubberBand();

    /**
     * Set the color for the rubberband
     *  @param color  The color used to render this rubberband
     */
    void setColor( const QColor & color );

    /**
     * Set the width of the line. Outline width for polygon.
     *  @param width The width for any lines painted for this rubberband
     */
    void setWidth( int width );

    /**
     * Set the icon type to highlight point geometries.
     *  @param icon The icon to visualize point geometries
     *  @note Added in 1.9
     */
    void setIcon( IconType icon );

    /**
     * Set the size of the point icons
     *  @note Added in 1.9
     */
    void setIconSize( int iconSize );

    /**
    * Set the style of the line
    *  @note Added in 1.9
    */
    void setLineStyle( Qt::PenStyle penStyle );

    /**
    * Set the style of the brush
    *  @note Added in 1.9
    */
    void setBrushStyle( Qt::BrushStyle brushStyle );

    /**
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to geometryType.
     *  @param geometryType Defines how the data should be drawn onto the screen. (Use QGis::Line, QGis::Polygon or QGis::Point)
     *  @note Added in 1.9.
     */
    void reset( QGis::GeometryType geometryType = QGis::Line );

    /**
     * @deprecated Use the reset method which takes QGis::GeometryType as second argument instead
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to isPolygon.
     *  @param isPolygon true: draw as (multi-)polygon, false draw as (multi-)linestring
     */
    void reset( bool isPolygon );

    /**
     * Add a vertex to the rubberband and update canvas.
     * The rendering of the vertex depends on the current GeometryType and icon.
     * If adding more points consider using update=false for better performance
     *  @param p             The vertex/point to add
     *  @param doUpdate      Should the map canvas be updated immediately?
     *  @param geometryIndex The index of the feature part (in case of multipart geometries)
     */
    void addPoint( const QgsPoint & p, bool doUpdate = true, int geometryIndex = 0 );

    /**
    * Remove a vertex from the rubberband and (optionally) update canvas.
    * @param index The index of the vertex/point to remove, negative indexes start at end
    * @param doUpdate Should the map canvas be updated immediately?
    * @param geometryIndex The index of the feature part (in case of multipart geometries)
    */
    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0 );

    /**
     * Removes the last point. Most useful in connection with undo operations
     */
    void removeLastPoint( int geometryIndex = 0 );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( const QgsPoint & p, int geometryIndex = 0 );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( int index, const QgsPoint& p, int geometryIndex = 0 );

    /**
     * Returns number of vertices in feature part
     *  @param geometryIndex The index of the feature part (in case of multipart geometries)
     *  @return number of vertices
     */
    int partSize( int geometryIndex ) const;

    /**
     * Sets this rubber band to the geometry of an existing feature.
     * This is useful for feature highlighting.
     * In contrast to {@link addGeometry}, this method does also change the geometry type of the rubberband.
     *  @param geom the geometry object
     *  @param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     */
    void setToGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**
     * Sets this rubber band to a map canvas rectangle
     *  @param rect rectangle in canvas coordinates
     *  @note added in version 1.7
     */
    void setToCanvasRectangle( const QRect& rect );

    /**
     * Add the geometry of an existing feature to a rubberband
     * This is useful for multi feature highlighting.
     * As of 2.0, this method does not change the GeometryType any more. You need to set the GeometryType
     * of the rubberband explicitly by calling {@link reset} or {@link setToGeometry} with appropriate arguments.
     * {@link setToGeometry} is also to be preferred for backwards-compatibility.
     *
     *  @param geom the geometry object. Will be treated as a collection of vertices.
     *  @param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     *  @note added in 1.5
     */
    void addGeometry( QgsGeometry* geom, QgsVectorLayer* layer );

    /**
     * Adds translation to original coordinates (all in map coordinates)
     *  @param dx  x translation
     *  @param dy  y translation
     */
    void setTranslationOffset( double dx, double dy );

    /**
     * Returns number of geometries
     *  @return number of geometries
     *  @note added in 1.5
     */
    int size() const;

    /**
     * Returns count of vertices in all lists of mPoint
     *  @return The total number of vertices
     */
    int numberOfVertices() const;

    /**
     * Return vertex
     *  @param i   The geometry index
     *  @param j   The vertex index within geometry i
     */
    const QgsPoint *getPoint( int i, int j = 0 ) const;

    /**
     * Returns the rubberband as a Geometry.
     *  @return A geometry object which reflects the current state of the rubberband.
     *  @note Added in 1.6
     */
    QgsGeometry* asGeometry();

  protected:
    virtual void paint( QPainter* p );

    //! recalculates needed rectangle
    void updateRect();

  private:
    QBrush mBrush;
    QPen mPen;

    /** The size of the icon for points.
      * @note Added in 1.9 */
    int mIconSize;

    /** Icon to be shown.
     *  @note Added in 1.9 */
    IconType mIconType;

    /**
     * Nested lists used for multitypes
     */
    QList< QList <QgsPoint> > mPoints;
    QGis::GeometryType mGeometryType;
    double mTranslationOffsetX;
    double mTranslationOffsetY;

    QgsRubberBand();

    static QgsPolyline getPolyline( const QList<QgsPoint> & points );

};

#endif
