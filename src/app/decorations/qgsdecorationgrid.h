/***************************************************************************
                         qgsdecorationgrid.h
                         ----------------------
    begin                : May 10, 2012
    copyright            : (C) 2012 by Etienne Tourigny
    email                : etourigny.dev at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONGRID_H
#define QGSDECORATIONGRID_H

#include "qgsdecorationitem.h"
#include "qgstextformat.h"

class QPainter;
class QgsLineSymbol;
class QgsMarkerSymbol;

#include <QColor>
#include <QPen>
#include <QFont>
#include "qgis_app.h"

class APP_EXPORT QgsDecorationGrid: public QgsDecorationItem
{

    Q_OBJECT

  public:

    QgsDecorationGrid( QObject *parent = nullptr );
    ~ QgsDecorationGrid() override;

    enum GridStyle
    {
      Line = 0, // lines
      Marker //markers
    };

    enum GridAnnotationDirection
    {
      Horizontal = 0,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    /**
     * Returns the title text format.
     * \see setTextFormat()
     * \see labelExtents()
     */
    QgsTextFormat textFormat() const { return mTextFormat; }

    /**
     * Sets the title text \a format.
     * \see textFormat()
     * \see setLabelExtents()
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

    //! Sets coordinate grid style.
    void setGridStyle( GridStyle style ) {mGridStyle = style;}
    GridStyle gridStyle() const { return mGridStyle; }

    //! Sets coordinate interval in x-direction for composergrid.
    void setGridIntervalX( double interval ) { mGridIntervalX = interval;}
    double gridIntervalX() const { return mGridIntervalX; }

    //! Sets coordinate interval in y-direction for composergrid.
    void setGridIntervalY( double interval ) { mGridIntervalY = interval;}
    double gridIntervalY() const { return mGridIntervalY; }

    //! Sets x-coordinate offset for composer grid
    void setGridOffsetX( double offset ) { mGridOffsetX = offset; }
    double gridOffsetX() const { return mGridOffsetX; }

    //! Sets y-coordinate offset for composer grid
    void setGridOffsetY( double offset ) { mGridOffsetY = offset; }
    double gridOffsetY() const { return mGridOffsetY; }

    //! Sets the pen to draw composer grid
    void setGridPen( const QPen &p ) { mGridPen = p; }
    QPen gridPen() const { return mGridPen; }
    //! Sets with of grid pen
    void setGridPenWidth( double w ) { mGridPen.setWidthF( w ); }
    //! Sets the color of the grid pen
    void setGridPenColor( const QColor &c ) {  mGridPen.setColor( c ); }

    //! Sets font for grid annotations
    void setGridAnnotationFont( const QFont &f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const { return mGridAnnotationFont; }

    //! Sets coordinate precision for grid annotations
    void setGridAnnotationPrecision( int p ) {mGridAnnotationPrecision = p;}
    int gridAnnotationPrecision() const {return mGridAnnotationPrecision;}

    //! Sets flag if grid annotation should be shown
    void setShowGridAnnotation( bool show ) {mShowGridAnnotation = show;}
    bool showGridAnnotation() const {return mShowGridAnnotation;}

    //! Sets distance between map frame and annotations
    void setAnnotationFrameDistance( double d ) {mAnnotationFrameDistance = d;}
    double annotationFrameDistance() const {return mAnnotationFrameDistance;}

    //! Sets grid annotation direction. Can be horizontal, vertical, direction of axis and horizontal and vertical
    void setGridAnnotationDirection( GridAnnotationDirection d ) {mGridAnnotationDirection = d;}
    GridAnnotationDirection gridAnnotationDirection() const {return mGridAnnotationDirection;}

    //! Sets symbol that is used to draw grid lines. Takes ownership
    void setLineSymbol( QgsLineSymbol *symbol );
    const QgsLineSymbol *lineSymbol() const { return mLineSymbol.get(); }

    //! Sets symbol that is used to draw markers. Takes ownership
    void setMarkerSymbol( QgsMarkerSymbol *symbol );
    const QgsMarkerSymbol *markerSymbol() const { return mMarkerSymbol.get(); }

    //! Sets map unit type
    void setMapUnits( QgsUnitTypes::DistanceUnit t ) { mMapUnits = t; }
    QgsUnitTypes::DistanceUnit mapUnits() const { return mMapUnits; }

    //! Sets mapUnits value
    void setDirty( bool dirty = true );
    bool isDirty();

    //! Computes interval that is approx. 1/5 of canvas extent
    bool getIntervalFromExtent( double *values, bool useXAxis = true ) const;
    //! Computes interval from current raster layer
    bool getIntervalFromCurrentLayer( double *values ) const;

  public slots:
    //! Sets values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! this does the meaty bit of the work
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;
    //! Show the dialog box
    void run() override;

    //! check that map units changed and disable if necessary
    void checkMapUnitsChanged();

  private:

    //! Enum for different frame borders
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    //! Line or Symbol
    GridStyle mGridStyle;
    //! Grid line interval in x-direction (map units)
    double mGridIntervalX;
    //! Grid line interval in y-direction (map units)
    double mGridIntervalY;
    //! Grid line offset in x-direction
    double mGridOffsetX;
    //! Grid line offset in y-direction
    double mGridOffsetY;
    //! Grid line pen
    QPen mGridPen;
    //! Font for grid line annotation
    QFont mGridAnnotationFont;
    //! Digits after the dot
    int mGridAnnotationPrecision;
    //! True if coordinate values should be drawn
    bool mShowGridAnnotation;
    //! Distance between map frame and annotation
    double mAnnotationFrameDistance;
    //! Annotation can be horizontal / vertical or different for axes
    GridAnnotationDirection mGridAnnotationDirection;

    std::unique_ptr< QgsLineSymbol > mLineSymbol;
    std::unique_ptr< QgsMarkerSymbol > mMarkerSymbol;

    QgsUnitTypes::DistanceUnit mMapUnits;

    /**
     * Draw coordinates for mGridAnnotationType Coordinate
     * \param p drawing painter
     * \param hLines horizontal coordinate lines in item coordinates
     * \param vLines vertical coordinate lines in item coordinates
     */
    void drawCoordinateAnnotations( QgsRenderContext &context, const QList< QPair< qreal, QLineF > > &hLines, const QList< QPair< qreal, QLineF > > &vLines );
    void drawCoordinateAnnotation( QgsRenderContext &context, QPointF pos, const QString &annotationString );

    /**
     * Returns the grid lines with associated coordinate value
     * \returns 0 in case of success
     */
    int xGridLines( const QgsMapSettings &mapSettings, QList< QPair< qreal, QLineF > > &lines ) const;

    /**
     * Returns the grid lines for the y-coordinates. Not vertical in case of rotation
     * \returns 0 in case of success
     */
    int yGridLines( const QgsMapSettings &mapSettings, QList< QPair< qreal, QLineF > > &lines ) const;

    //! Returns the item border of a point (in item coordinates)
    Border borderForLineCoord( QPointF point, const QPainter *p ) const;

    QgsTextFormat mTextFormat;
};

#endif
