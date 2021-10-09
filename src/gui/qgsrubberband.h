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
#include "qgscoordinatetransform.h"

#include <QBrush>
#include <QVector>
#include <QPen>
#include <QPolygon>
#include <QObject>
#include <QSvgRenderer>

#include "qgis_gui.h"

class QgsVectorLayer;
class QPaintEvent;
class QgsSymbol;

#ifdef SIP_RUN
% ModuleHeaderCode
// For ConvertToSubClassCode.
#include <qgsrubberband.h>
% End
#endif

/**
 * \ingroup gui
 * \brief A class for drawing transient features (e.g. digitizing lines) on the map.
 *
 * The QgsRubberBand class provides a transparent overlay widget
 * for tracking the mouse while drawing polylines or polygons.
 */
#ifndef SIP_RUN
class GUI_EXPORT QgsRubberBand : public QObject, public QgsMapCanvasItem
{
#else
class GUI_EXPORT QgsRubberBand : public QgsMapCanvasItem
{
#endif
    Q_OBJECT

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( dynamic_cast<QgsRubberBand *>( sipCpp ) )
      sipType = sipType_QgsRubberBand;
    else
      sipType = nullptr;
    SIP_END
#endif
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

      /**
       * An svg image is used to highlight points
       * \since QGIS 3.10
       */
      ICON_SVG
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
    ~QgsRubberBand() override;

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
     * Set the path to the svg file to use to draw points.
     * Calling this function automatically calls setIcon(ICON_SVG)
     * \param path The path to the svg
     * \param drawOffset The offset where to draw the image origin
     * \since QGIS 3.10
     */
    void setSvgIcon( const QString &path, QPoint drawOffset );


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
     * If adding more points consider using update=FALSE for better performance
     *  \param p             The vertex/point to add
     *  \param doUpdate      Should the map canvas be updated immediately?
     *  \param geometryIndex The index of the feature part (in case of multipart geometries)
     *  \param ringIndex     The index of the polygon ring (in case of polygons with holes)
     */
    void addPoint( const QgsPointXY &p, bool doUpdate = true, int geometryIndex = 0, int ringIndex = 0 );

    /**
     * Ensures that a polygon geometry is closed and that the last vertex equals the
     * first vertex.
     * \param doUpdate set to TRUE to update the map canvas immediately
     * \param geometryIndex The index of the feature part (in case of multipart geometries)
     * \param ringIndex     The index of the polygon ring (in case of polygons with holes)
     * \since QGIS 2.16
     */
    void closePoints( bool doUpdate = true, int geometryIndex = 0, int ringIndex = 0 );

    /**
     * Removes a vertex from the rubberband and (optionally) updates canvas.
     * \param index The index of the vertex/point to remove, negative indexes start at end
     * \param doUpdate Should the map canvas be updated immediately?
     * \param geometryIndex The index of the feature part (in case of multipart geometries)
     * \param ringIndex     The index of the polygon ring (in case of polygons with holes)
     */
    void removePoint( int index = 0, bool doUpdate = true, int geometryIndex = 0, int ringIndex = 0 );

    /**
     * Removes the last point. Most useful in connection with undo operations
     */
    void removeLastPoint( int geometryIndex = 0, bool doUpdate = true, int ringIndex = 0 );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( const QgsPointXY &p, int geometryIndex = 0, int ringIndex = 0 );

    /**
     * Moves the rubber band point specified by index. Note that if the rubber band is
     * not used to track the last mouse position, the first point of the rubber band has two vertices
     */
    void movePoint( int index, const QgsPointXY &p, int geometryIndex = 0, int ringIndex = 0 );

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
     *               crs. If \a layer is NULLPTR, the coordinates are not going to be transformed.
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
     * Copies the points from another rubber band.
     *
     * \since QGIS 3.22
     */
    void copyPointsFrom( const QgsRubberBand *other );

    /**
     * Adds the geometry of an existing feature to a rubberband
     * This is useful for multi feature highlighting.
     * As of 2.0, this method does not change the GeometryType any more. You need to set the GeometryType
     * of the rubberband explicitly by calling reset() or setToGeometry() with appropriate arguments.
     * setToGeometry() is also to be preferred for backwards-compatibility.
     *
     * If additional geometries are to be added then set \a doUpdate to FALSE to defer costly repaint and bounding rectangle calculations for better performance.
     * After adding the final geometry updatePosition() should be called.
     *
     *  \param geometry the geometry object. Will be treated as a collection of vertices.
     *  \param layer the layer associated with the geometry. This is used for transforming the geometry from the layer's CRS to the map crs. If \a layer is NULLPTR no coordinate transformation will occur.
     *  \param doUpdate set to FALSE to defer updates of the rubber band.
     */
    void addGeometry( const QgsGeometry &geometry, QgsMapLayer *layer, bool doUpdate = true );

    /**
     * Adds a \a geometry to the rubberband.
     *
     * If \a crs is specified, the geometry will be automatically reprojected from \a crs
     * to the canvas CRS.
     *
     * If additional geometries are to be added then set \a doUpdate to FALSE to defer costly repaint and bounding rectangle calculations for better performance.
     * After adding the final geometry updatePosition() should be called.
     *
     * \since QGIS 3.0
     */
    void addGeometry( const QgsGeometry &geometry, const QgsCoordinateReferenceSystem &crs = QgsCoordinateReferenceSystem(), bool doUpdate = true );

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

    // TODO QGIS 4: rename i to geometryIndex, j to vertexIndex
    // TODO QGIS 4: reorder parameters to geom, ring, ring

    /**
     * Returns a vertex
     *  \param i   The geometry index
     *  \param j   The vertex index within ring ringIndex
     *  \param ringIndex   The ring index within geometry i
     */
    const QgsPointXY *getPoint( int i, int j = 0, int ringIndex = 0 ) const;

    /**
     * Returns the rubberband as a Geometry
     *  \returns A geometry object which reflects the current state of the rubberband.
     */
    QgsGeometry asGeometry() const;

    void updatePosition() override;

    /**
     * Returns the symbol used for rendering the rubberband, if set.
     *
     * \see setSymbol()
     * \since QGIS 3.20
     */
    QgsSymbol *symbol() const;

    /**
     * Sets the \a symbol used for rendering the rubberband.
     *
     * Ownership of \a symbol is transferred to the rubberband.
     *
     * \warning Only line and fill symbols are currently supported.
     *
     * \note Setting a symbol for the rubberband overrides any other appearance setting,
     * such as the strokeColor() or width().
     *
     * \see setSymbol()
     * \since QGIS 3.20
     */
    void setSymbol( QgsSymbol *symbol SIP_TRANSFER );

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

    /**
     * Draws shape of the rubber band.
     *  \param p The QPainter object
     *  \param rings A list of points used to draw the shape
     */
    void drawShape( QPainter *p, const QVector<QPolygonF> &rings );

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
    std::unique_ptr<QSvgRenderer> mSvgRenderer;
    QPoint mSvgOffset;

    std::unique_ptr< QgsSymbol > mSymbol;

    /**
     * Nested lists used for multitypes
     */
    QVector< QVector< QVector <QgsPointXY> > > mPoints;
    QgsWkbTypes::GeometryType mGeometryType = QgsWkbTypes::PolygonGeometry;
    double mTranslationOffsetX = 0.0;
    double mTranslationOffsetY = 0.0;

    QgsRubberBand();

};

#endif
