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
#include "qgis_sip.h"
#include "qgsgeometry.h"

#include <QBrush>
#include <QList>
#include <QPen>
#include <QPolygon>
#include <QObject>

#include "qgis_gui.h"

class QgsVectorLayer;
class QPaintEvent;

/**
 * \ingroup gui
 * A class for drawing transient features (e.g. digitizing lines) on the map.
 *
 * The QgsRubberBand class provides a transparent overlay widget
 * for tracking the mouse while drawing polylines or polygons.
 */
class GUI_EXPORT QgsRubberBand : public QObject, public QgsMapCanvasItem
{
    Q_OBJECT
  public:

    Q_PROPERTY( QColor fillColor READ fillColor WRITE setFillColor )
    Q_PROPERTY( QColor strokeColor READ strokeColor WRITE setStrokeColor )
    Q_PROPERTY( int iconSize READ iconSize WRITE setIconSize )
    Q_PROPERTY( QColor secondaryStrokeColor READ secondaryStrokeColor WRITE setSecondaryStrokeColor )
    Q_PROPERTY( int width READ width WRITE setWidth )

    //! Icons
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
      ICON_CIRCLE,

      /**
       * A full box is used to highlight points (■)
       */
      ICON_FULL_BOX,

      /**
       * A diamond is used to highlight points (◇)
       * \since QGIS 3.0
       */
      ICON_DIAMOND,

      /**
       * A diamond is used to highlight points (◆)
       * \since QGIS 3.0
       */
      ICON_FULL_DIAMOND,
    };

    /**
     * Creates a new RubberBand.
     *  \param mapCanvas The map canvas to draw onto.
     *         Its CRS will be used to map points onto screen coordinates.
     * The ownership is transferred to this canvas.
     *  \param geometryType Defines how the data should be drawn onto the screen.
     *         QgsWkbTypes::LineGeometry, QgsWkbTypes::PolygonGeometry or QgsWkbTypes::PointGeometry
     */
    QgsRubberBand( QgsMapCanvas *mapCanvas SIP_TRANSFERTHIS, QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry );

    /**
     * Sets the color for the rubberband.
     * Shorthand method to set fill and stroke color with a single call.
     *  \param color  The color used to render this rubberband
     */
    void setColor( const QColor &color );

    /**
     * Sets the fill color for the rubberband
     *  \param color  The color used to render this rubberband
     *  \since QGIS 2.6
     */
    void setFillColor( const QColor &color );

    /**
     * Returns the current fill color.
     */
    QColor fillColor() const { return mBrush.color(); }

    /**
     * Sets the stroke color for the rubberband
     *  \param color  The color used to render this rubberband
     *  \since QGIS 2.6
     */
    void setStrokeColor( const QColor &color );

    /**
     * Returns the current stroke color.
     */
    QColor strokeColor() const { return mPen.color(); }

    /**
     * Sets a secondary stroke color for the rubberband which will be drawn under the main stroke color.
     * Set to an invalid color to avoid drawing the secondary stroke.
     *  \param color  The color used to render a secondary stroke color to this rubberband
     *  \since QGIS 3.0
     */
    void setSecondaryStrokeColor( const QColor &color );

    /**
     * Returns the current secondary stroke color.
     */
    QColor secondaryStrokeColor() const { return mSecondaryPen.color(); }

    /**
     * Sets the width of the line. Stroke width for polygon.
     *  \param width The width for any lines painted for this rubberband
     */
    void setWidth( int width );

    /**
     * Returns the current width of the line or stroke width for polygon.
     */
    int width() const { return mPen.width(); }

    /**
     * Sets the icon type to highlight point geometries.
     *  \param icon The icon to visualize point geometries
     */
    void setIcon( IconType icon );


    /**
     * Returns the current icon type to highlight point geometries.
     */
    IconType icon() const { return mIconType; }

    /**
     * Sets the size of the point icons
     */
    void setIconSize( int iconSize );

    /**
     * Returns the current icon size of the point icons.
     */
    int iconSize() const { return mIconSize; }

    /**
     * Sets the style of the line
     */
    void setLineStyle( Qt::PenStyle penStyle );

    /**
     * Sets the style of the brush
     */
    void setBrushStyle( Qt::BrushStyle brushStyle );

    /**
     * Clears all the geometries in this rubberband.
     * Sets the representation type according to geometryType.
     *  \param geometryType Defines how the data should be drawn onto the screen. (Use Qgis::Line, Qgis::Polygon or Qgis::Point)
     */
    void reset( QgsWkbTypes::GeometryType geometryType = QgsWkbTypes::LineGeometry );

    /**
     * Adds a vertex to the rubberband and update canvas.
     * The rendering of the vertex depends on the current GeometryType and icon.
     * If adding more points consider using update=false for better performance
     *  \param p             The vertex/point to add
     *  \param doUpdate      Should the map canvas be updated immediately?
     *  \param geometryIndex The index of the feature part (in case of multipart geometries)
     */
    void addPoint( const QgsPointXY &p, bool doUpdate = true, int geometryIndex = 0 );

    /**
     * Ensures that a polygon geometry is closed and that the last vertex equals the
     * first vertex.
     * \param doUpdate set to true to update the map canvas immediately
     * \param geometryIndex index of the feature part (in case of multipart geometries)
     * \since QGIS 2.16
     */
    void closePoints( bool doUpdate = true, int geometryIndex = 0 );

    /**
     * Removes a vertex from the rubberband and (optionally) updates canvas.
     * \param index The index of the vertex/point to remove, negative indexes start at end
     * \param doUpdate Should the map canvas be updated immediately?
     * \param geometryIndex The index of the feature part (in case of multipart geometries)
     */
    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0 );

    /**
     * Removes the last point. Most useful in connection with undo operations
     */
    void removeLastPoint( int geometryIndex = 0, bool doUpdate = true );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( const QgsPointXY &p, int geometryIndex = 0 );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( int index, const QgsPointXY &p, int geometryIndex = 0 );

    /**
     * Returns number of vertices in feature part
     *  \param geometryIndex The index of the feature part (in case of multipart geometries)
     *  \returns number of vertices
     */
    int partSize( int geometryIndex ) const;

    /**
     * Sets this rubber band to \a geom.
     * This is useful for feature highlighting.
     * In contrast to addGeometry(), this method does also change the geometry type of the rubberband.
     *  \param geom the geometry object
     *  \param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     */
    void setToGeometry( const QgsGeometry &geom, QgsVectorLayer *layer );

    /**
     * Sets this rubber band to \a geometry.
     * In contrast to addGeometry(), this method does also change the geometry type of the rubberband.
     * The coordinate reference system of the geometry can be specified with \a crs. If an invalid \a crs
     * is passed, the geometry will not be reprojected and needs to be in canvas crs already.
     * By default, no reprojection is done.
     *
     * \since QGIS 3.4
     */
    void setToGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Sets this rubber band to a map canvas rectangle
     *  \param rect rectangle in canvas coordinates
     */
    void setToCanvasRectangle( QRect rect );

    /**
     * Adds the geometry of an existing feature to a rubberband
     * This is useful for multi feature highlighting.
     * As of 2.0, this method does not change the GeometryType any more. You need to set the GeometryType
     * of the rubberband explicitly by calling reset() or setToGeometry() with appropriate arguments.
     * setToGeometry() is also to be preferred for backwards-compatibility.
     *
     *  \param geometry the geometry object. Will be treated as a collection of vertices.
     *  \param layer the layer containing the feature, used for coord transformation to map
     *               crs. In case of 0 pointer, the coordinates are not going to be transformed.
     */
    void addGeometry( const QgsGeometry &geometry, QgsVectorLayer *layer );

    /**
     * Adds a \a geometry to the rubberband.
     *
     * If \a crs is specified, the geometry will be automatically reprojected from \a crs
     * to the canvas CRS.
     *
     * \since QGIS 3.0
     */
    void addGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem() );

    /**
     * Adds translation to original coordinates (all in map coordinates)
     *  \param dx  x translation
     *  \param dy  y translation
     */
    void setTranslationOffset( double dx, double dy );

    /**
     * Returns number of geometries
     *  \returns number of geometries
     */
    int size() const;

    /**
     * Returns count of vertices in all lists of mPoint
     *  \returns The total number of vertices
     */
    int numberOfVertices() const;

    /**
     * Returns a vertex
     *  \param i   The geometry index
     *  \param j   The vertex index within geometry i
     */
    const QgsPointXY *getPoint( int i, int j = 0 ) const;

    /**
     * Returns the rubberband as a Geometry
     *  \returns A geometry object which reflects the current state of the rubberband.
     */
    QgsGeometry asGeometry() const;

    void updatePosition() override;

  protected:

    /**
     * Paints the rubber band in response to an update event.
     *  \param p The QPainter object
     */
    void paint( QPainter *p ) override;

    /**
     * Draws shape of the rubber band.
     *  \param p The QPainter object
     *  \param pts A list of points used to draw the shape
     */
    void drawShape( QPainter *p, const QVector<QPointF> &pts );

    //! Recalculates needed rectangle
    void updateRect();

  private:
    QBrush mBrush;
    QPen mPen;
    QPen mSecondaryPen;

    //! The size of the icon for points.
    int mIconSize = 5;

    //! Icon to be shown.
    IconType mIconType = ICON_CIRCLE;

    /**
     * Nested lists used for multitypes
     */
    QList< QList <QgsPointXY> > mPoints;
    QgsWkbTypes::GeometryType mGeometryType = QgsWkbTypes::PolygonGeometry;
    double mTranslationOffsetX = 0.0;
    double mTranslationOffsetY = 0.0;

    QgsRubberBand();

    static QgsPolylineXY getPolyline( const QList<QgsPointXY> &points );

};

#endif
