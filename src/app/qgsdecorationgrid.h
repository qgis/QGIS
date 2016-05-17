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
#include <qgis.h>

class QPainter;
class QgsLineSymbolV2;
class QgsMarkerSymbolV2;

#include <QColor>
#include <QPen>
#include <QFont>

class APP_EXPORT QgsDecorationGrid: public QgsDecorationItem
{
    Q_OBJECT
  public:
    //! Constructor
    QgsDecorationGrid( QObject* parent = nullptr );
    //! Destructor
    virtual ~ QgsDecorationGrid();

    enum GridStyle
    {
      Line = 0, //solid lines
      //      Cross = 1, //only draw line crossings
      Marker = 1 //user-defined marker
    };

    enum GridAnnotationPosition
    {
      InsideMapFrame = 0,
      OutsideMapFrame
    };

    enum GridAnnotationDirection
    {
      Horizontal = 0,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    /** Sets coordinate grid style. */
    void setGridStyle( GridStyle style ) {mGridStyle = style;}
    GridStyle gridStyle() const { return mGridStyle; }

    /** Sets coordinate interval in x-direction for composergrid. */
    void setGridIntervalX( double interval ) { mGridIntervalX = interval;}
    double gridIntervalX() const { return mGridIntervalX; }

    /** Sets coordinate interval in y-direction for composergrid. */
    void setGridIntervalY( double interval ) { mGridIntervalY = interval;}
    double gridIntervalY() const { return mGridIntervalY; }

    /** Sets x-coordinate offset for composer grid */
    void setGridOffsetX( double offset ) { mGridOffsetX = offset; }
    double gridOffsetX() const { return mGridOffsetX; }

    /** Sets y-coordinate offset for composer grid */
    void setGridOffsetY( double offset ) { mGridOffsetY = offset; }
    double gridOffsetY() const { return mGridOffsetY; }

    /** Sets the pen to draw composer grid */
    void setGridPen( const QPen& p ) { mGridPen = p; }
    QPen gridPen() const { return mGridPen; }
    /** Sets with of grid pen */
    void setGridPenWidth( double w ) { mGridPen.setWidthF( w ); }
    /** Sets the color of the grid pen */
    void setGridPenColor( const QColor& c ) {  mGridPen.setColor( c ); }

    /** Sets font for grid annotations */
    void setGridAnnotationFont( const QFont& f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const { return mGridAnnotationFont; }

    /** Sets coordinate precision for grid annotations */
    void setGridAnnotationPrecision( int p ) {mGridAnnotationPrecision = p;}
    int gridAnnotationPrecision() const {return mGridAnnotationPrecision;}

    /** Sets flag if grid annotation should be shown */
    void setShowGridAnnotation( bool show ) {mShowGridAnnotation = show;}
    bool showGridAnnotation() const {return mShowGridAnnotation;}

    /** Sets position of grid annotations. Possibilities are inside or outside of the map frame */
    void setGridAnnotationPosition( GridAnnotationPosition p ) {mGridAnnotationPosition = p;}
    GridAnnotationPosition gridAnnotationPosition() const {return mGridAnnotationPosition;}

    /** Sets distance between map frame and annotations */
    void setAnnotationFrameDistance( double d ) {mAnnotationFrameDistance = d;}
    double annotationFrameDistance() const {return mAnnotationFrameDistance;}

    /** Sets grid annotation direction. Can be horizontal, vertical, direction of axis and horizontal and vertical */
    void setGridAnnotationDirection( GridAnnotationDirection d ) {mGridAnnotationDirection = d;}
    GridAnnotationDirection gridAnnotationDirection() const {return mGridAnnotationDirection;}

    /** Sets length of the cros segments (if grid style is cross) */
    /* void setCrossLength( double l ) {mCrossLength = l;} */
    /* double crossLength() {return mCrossLength;} */

    /** Set symbol that is used to draw grid lines. Takes ownership*/
    void setLineSymbol( QgsLineSymbolV2* symbol );
    const QgsLineSymbolV2* lineSymbol() const { return mLineSymbol; }

    /** Set symbol that is used to draw markers. Takes ownership*/
    void setMarkerSymbol( QgsMarkerSymbolV2* symbol );
    const QgsMarkerSymbolV2* markerSymbol() const { return mMarkerSymbol; }

    /** Sets map unit type */
    void setMapUnits( QGis::UnitType t ) { mMapUnits = t; }
    QGis::UnitType mapUnits() { return mMapUnits; }

    /** Set mapUnits value */
    void setDirty( bool dirty = true );
    bool isDirty();

    /** Computes interval that is approx. 1/5 of canvas extent */
    bool getIntervalFromExtent( double* values, bool useXAxis = true );
    /** Computes interval from current raster layer */
    bool getIntervalFromCurrentLayer( double* values );

    double getDefaultInterval( bool useXAxis = true );

  public slots:
    //! set values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! this does the meaty bit of the work
    void render( QPainter * ) override;
    //! Show the dialog box
    void run() override;

    //! check that map units changed and disable if necessary
    void checkMapUnitsChanged();

  private:

    /** Enum for different frame borders*/
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    /** Line or Symbol */
    GridStyle mGridStyle;
    /** Grid line interval in x-direction (map units)*/
    double mGridIntervalX;
    /** Grid line interval in y-direction (map units)*/
    double mGridIntervalY;
    /** Grid line offset in x-direction*/
    double mGridOffsetX;
    /** Grid line offset in y-direction*/
    double mGridOffsetY;
    /** Grid line pen*/
    QPen mGridPen;
    /** Font for grid line annotation*/
    QFont mGridAnnotationFont;
    /** Digits after the dot*/
    int mGridAnnotationPrecision;
    /** True if coordinate values should be drawn*/
    bool mShowGridAnnotation;
    /** Annotation position inside or outside of map frame*/
    GridAnnotationPosition mGridAnnotationPosition;
    /** Distance between map frame and annotation*/
    double mAnnotationFrameDistance;
    /** Annotation can be horizontal / vertical or different for axes*/
    GridAnnotationDirection mGridAnnotationDirection;
    /** The length of the cross sides for mGridStyle Cross*/
    /* double mCrossLength; */

    QgsLineSymbolV2* mLineSymbol;
    QgsMarkerSymbolV2* mMarkerSymbol;

    QGis::UnitType mMapUnits;

    /** Draw coordinates for mGridAnnotationType Coordinate
        @param p drawing painter
    @param hLines horizontal coordinate lines in item coordinates
        @param vLines vertical coordinate lines in item coordinates*/
    void drawCoordinateAnnotations( QPainter* p, const QList< QPair< qreal, QLineF > >& hLines, const QList< QPair< qreal, QLineF > >& vLines );
    void drawCoordinateAnnotation( QPainter* p, QPointF pos, const QString& annotationString );
    /** Draws a single annotation
        @param p drawing painter
        @param pos item coordinates where to draw
        @param rotation text rotation
        @param annotationText the text to draw*/
    void drawAnnotation( QPainter* p, QPointF pos, int rotation, const QString& annotationText );
    /** Returns the grid lines with associated coordinate value
        @return 0 in case of success*/
    int xGridLines( QList< QPair< qreal, QLineF > >& lines ) const;
    /** Returns the grid lines for the y-coordinates. Not vertical in case of rotation
        @return 0 in case of success*/
    int yGridLines( QList< QPair< qreal, QLineF > >& lines ) const;
    /** Returns the item border of a point (in item coordinates)*/
    Border borderForLineCoord( QPointF point, QPainter* p ) const;

    /** Draws Text. Takes care about all the composer specific issues (calculation to pixel, scaling of font and painter
     to work around the Qt font bug)*/
    void drawText( QPainter* p, double x, double y, const QString& text, const QFont& font ) const;
    /** Like the above, but with a rectangle for multiline text*/
    void drawText( QPainter* p, const QRectF& rect, const QString& text, const QFont& font, Qt::AlignmentFlag halignment = Qt::AlignLeft, Qt::AlignmentFlag valignment = Qt::AlignTop ) const;
    /** Returns the font width in millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double textWidthMillimeters( const QFont& font, const QString& text ) const;
    /** Returns the font height of a character in millimeters. */
    double fontHeightCharacterMM( const QFont& font, QChar c ) const;
    /** Returns the font ascent in Millimeters (considers upscaling and downscaling with FONT_WORKAROUND_SCALE*/
    double fontAscentMillimeters( const QFont& font ) const;
    /** Calculates font to from point size to pixel size*/
    double pixelFontSize( double pointSize ) const;
    /** Returns a font where size is in pixel and font size is upscaled with FONT_WORKAROUND_SCALE*/
    QFont scaledFontPixelSize( const QFont& font ) const;

    /* friend class QgsDecorationGridDialog; */
};

#endif
