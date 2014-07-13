/***************************************************************************
                         qgscomposermapgrid.h
                         --------------------
    begin                : December 2013
    copyright            : (C) 2013 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERMAPGRID_H
#define QGSCOMPOSERMAPGRID_H

#include "qgscoordinatereferencesystem.h"
#include "qgscomposermap.h"
#include <QString>

class QgsCoordinateTransform;
class QgsLineSymbolV2;
class QgsMarkerSymbolV2;
class QDomDocument;
class QDomElement;
class QPainter;

class CORE_EXPORT QgsComposerMapGrid
{
  public:

    enum GridUnit
    {
      MapUnit,
      MM,
      CM
    };

    QgsComposerMapGrid( const QString& name, QgsComposerMap* map );
    ~QgsComposerMapGrid();

    /** \brief Reimplementation of QCanvasItem::paint*/
    void drawGrid( QPainter* painter ) const;

    /** stores state in Dom element
       * @param elem is Dom element corresponding to 'ComposerMap' tag
       * @param doc Dom document
       */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const;

    /** sets state from Dom document
       * @param itemElem is Dom node corresponding to item tag
       * @param doc is Dom document
       */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc );

    //setters and getters
    void setComposerMap( QgsComposerMap* map );
    const QgsComposerMap* composerMap() const { return mComposerMap; }

    void setName( const QString& name ) { mName = name; }
    QString name() const { return mName; }

    QString id() const { return mUuid; }

    /**Enables a coordinate grid that is shown on top of this composermap.
        @note this function was added in version 1.4*/
    void setGridEnabled( bool enabled ) {mGridEnabled = enabled;}
    bool gridEnabled() const { return mGridEnabled; }

    /**Sets coordinate grid style to solid or cross
        @note this function was added in version 1.4*/
    void setGridStyle( QgsComposerMap::GridStyle style ) {mGridStyle = style;}
    QgsComposerMap::GridStyle gridStyle() const { return mGridStyle; }

    /**Sets coordinate interval in x-direction for composergrid.
        @note this function was added in version 1.4*/
    void setGridIntervalX( double interval ) { mGridIntervalX = interval;}
    double gridIntervalX() const { return mGridIntervalX; }

    /**Sets coordinate interval in y-direction for composergrid.
    @note this function was added in version 1.4*/
    void setGridIntervalY( double interval ) { mGridIntervalY = interval;}
    double gridIntervalY() const { return mGridIntervalY; }

    /**Sets x-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetX( double offset ) { mGridOffsetX = offset; }
    double gridOffsetX() const { return mGridOffsetX; }

    /**Sets y-coordinate offset for composer grid
    @note this function was added in version 1.4*/
    void setGridOffsetY( double offset ) { mGridOffsetY = offset; }
    double gridOffsetY() const { return mGridOffsetY; }

    /**Sets the pen to draw composer grid
    @note this function was added in version 1.4*/
    void setGridPen( const QPen& p );
    QPen gridPen() const;

    /**Sets with of grid pen
    @note this function was added in version 1.4*/
    void setGridPenWidth( double w );

    /**Sets the color of the grid pen
    @note this function was added in version 1.4*/
    void setGridPenColor( const QColor& c );

    /**Sets font for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationFont( const QFont& f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const { return mGridAnnotationFont; }

    void setGridAnnotationFontColor( const QColor& c ) { mGridAnnotationFontColor = c; }
    QColor gridAnnotationFontColor() const { return mGridAnnotationFontColor; }

    /**Sets coordinate precision for grid annotations
    @note this function was added in version 1.4*/
    void setGridAnnotationPrecision( int p ) {mGridAnnotationPrecision = p;}
    int gridAnnotationPrecision() const {return mGridAnnotationPrecision;}

    /**Sets flag if grid annotation should be shown
    @note this function was added in version 1.4*/
    void setShowGridAnnotation( bool show ) {mShowGridAnnotation = show;}
    bool showGridAnnotation() const {return mShowGridAnnotation;}

    void setGridAnnotationPosition( QgsComposerMap::GridAnnotationPosition p, QgsComposerMap::Border border );
    QgsComposerMap::GridAnnotationPosition gridAnnotationPosition( QgsComposerMap::Border border ) const;

    /**Sets distance between map frame and annotations
    @note this function was added in version 1.4*/
    void setAnnotationFrameDistance( double d ) {mAnnotationFrameDistance = d;}
    double annotationFrameDistance() const {return mAnnotationFrameDistance;}

    void setGridAnnotationDirection( QgsComposerMap::GridAnnotationDirection d, QgsComposerMap::Border border );
    QgsComposerMap::GridAnnotationDirection gridAnnotationDirection( QgsComposerMap::Border border ) const;

    /**Sets grid annotation direction. Can be horizontal, vertical, direction of axis and horizontal and vertical
        @note provides compatibility with 1.8 version*/
    void setGridAnnotationDirection( QgsComposerMap::GridAnnotationDirection d );
    QgsComposerMap::GridAnnotationDirection gridAnnotationDirection() const;

    void setGridAnnotationFormat( QgsComposerMap::GridAnnotationFormat f ) { mGridAnnotationFormat = f; }
    QgsComposerMap::GridAnnotationFormat gridAnnotationFormat() const { return mGridAnnotationFormat; }

    /**Set grid frame style (NoGridFrame or Zebra)
        @note: this function was added in version 1.9*/
    void setGridFrameStyle( QgsComposerMap::GridFrameStyle style ) { mGridFrameStyle = style; }
    QgsComposerMap::GridFrameStyle gridFrameStyle() const { return mGridFrameStyle; }

    /**Set grid frame width
        @note: this function was added in version 1.9*/
    void setGridFrameWidth( double w ) { mGridFrameWidth = w; }
    double gridFrameWidth() const { return mGridFrameWidth; }

    /**Set grid frame pen thickness
            @note: this function was added in version 2.1*/
    void setGridFramePenSize( double w ) { mGridFramePenThickness = w; }
    double gridFramePenSize() const { return mGridFramePenThickness; }

    /**Sets pen color for grid frame
        @note: this function was added in version 2.1*/
    void setGridFramePenColor( const QColor& c ) { mGridFramePenColor = c;}
    /**Get pen color for grid frame
        @note: this function was added in version 2.1*/
    QColor gridFramePenColor() const {return mGridFramePenColor;}

    /**Sets first fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    void setGridFrameFillColor1( const QColor& c ) { mGridFrameFillColor1 = c;}
    /**Get first fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    QColor gridFrameFillColor1() const {return mGridFrameFillColor1;}

    /**Sets second fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    void setGridFrameFillColor2( const QColor& c ) { mGridFrameFillColor2 = c;}
    /**Get second fill color for grid zebra frame
        @note: this function was added in version 2.1*/
    QColor gridFrameFillColor2() const {return mGridFrameFillColor2;}

    /**Sets length of the cros segments (if grid style is cross)
    @note this function was added in version 1.4*/
    void setCrossLength( double l ) {mCrossLength = l;}
    double crossLength() const {return mCrossLength;}

    void setGridLineSymbol( QgsLineSymbolV2* symbol );
    const QgsLineSymbolV2* gridLineSymbol() const { return mGridLineSymbol; }
    QgsLineSymbolV2* gridLineSymbol() { return mGridLineSymbol; }

    void setGridMarkerSymbol( QgsMarkerSymbolV2* symbol );
    const QgsMarkerSymbolV2* gridMarkerSymbol() const { return mGridMarkerSymbol; }
    QgsMarkerSymbolV2* gridMarkerSymbol() { return mGridMarkerSymbol; }

    void setCrs( const QgsCoordinateReferenceSystem& crs ) { mCRS = crs; }
    QgsCoordinateReferenceSystem crs() const { return mCRS; }

    void setGridUnit( GridUnit u ) { mGridUnit = u; }
    GridUnit gridUnit() const { return mGridUnit; }

    void setBlendMode( QPainter::CompositionMode mode ) { mBlendMode = mode; }
    QPainter::CompositionMode blendMode() const { return mBlendMode; }

    double maxExtension() const;

  private:
    QgsComposerMapGrid(); //forbidden

    QgsComposerMap* mComposerMap;
    QString mName;
    QString mUuid;

    /**True if coordinate grid has to be displayed*/
    bool mGridEnabled;
    /**Solid or crosses*/
    QgsComposerMap::GridStyle mGridStyle;
    /**Grid line interval in x-direction (map units)*/
    double mGridIntervalX;
    /**Grid line interval in y-direction (map units)*/
    double mGridIntervalY;
    /**Grid line offset in x-direction*/
    double mGridOffsetX;
    /**Grid line offset in y-direction*/
    double mGridOffsetY;
    /**Font for grid line annotation*/
    QFont mGridAnnotationFont;
    /**Font color for grid coordinates*/
    QColor mGridAnnotationFontColor;
    /**Digits after the dot*/
    int mGridAnnotationPrecision;
    /**True if coordinate values should be drawn*/
    bool mShowGridAnnotation;

    /**Annotation position for left map side (inside / outside / not shown)*/
    QgsComposerMap::GridAnnotationPosition mLeftGridAnnotationPosition;
    /**Annotation position for right map side (inside / outside / not shown)*/
    QgsComposerMap::GridAnnotationPosition mRightGridAnnotationPosition;
    /**Annotation position for top map side (inside / outside / not shown)*/
    QgsComposerMap::GridAnnotationPosition mTopGridAnnotationPosition;
    /**Annotation position for bottom map side (inside / outside / not shown)*/
    QgsComposerMap::GridAnnotationPosition mBottomGridAnnotationPosition;

    /**Distance between map frame and annotation*/
    double mAnnotationFrameDistance;

    /**Annotation direction on left side ( horizontal or vertical )*/
    QgsComposerMap::GridAnnotationDirection mLeftGridAnnotationDirection;
    /**Annotation direction on right side ( horizontal or vertical )*/
    QgsComposerMap::GridAnnotationDirection mRightGridAnnotationDirection;
    /**Annotation direction on top side ( horizontal or vertical )*/
    QgsComposerMap::GridAnnotationDirection mTopGridAnnotationDirection;
    /**Annotation direction on bottom side ( horizontal or vertical )*/
    QgsComposerMap::GridAnnotationDirection mBottomGridAnnotationDirection;
    QgsComposerMap::GridAnnotationFormat mGridAnnotationFormat;
    QgsComposerMap::GridFrameStyle mGridFrameStyle;
    double mGridFrameWidth;
    double mGridFramePenThickness;
    QColor mGridFramePenColor;
    QColor mGridFrameFillColor1;
    QColor mGridFrameFillColor2;
    double mCrossLength;

    QgsLineSymbolV2* mGridLineSymbol;
    QgsMarkerSymbolV2* mGridMarkerSymbol;

    QgsCoordinateReferenceSystem mCRS;

    GridUnit mGridUnit;

    QPainter::CompositionMode mBlendMode;

    /**Draws the map grid*/
    void drawGridFrame( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const;
    /**Draw coordinates for mGridAnnotationType Coordinate
        @param p drawing painter
        @param hLines horizontal coordinate lines in item coordinates
        @param vLines vertical coordinate lines in item coordinates*/
    void drawCoordinateAnnotations( QPainter* p, const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines ) const;
    void drawCoordinateAnnotation( QPainter* p, const QPointF& pos, QString annotationString ) const;
    /**Draws a single annotation
        @param p drawing painter
        @param pos item coordinates where to draw
        @param rotation text rotation
        @param annotationText the text to draw*/
    void drawAnnotation( QPainter* p, const QPointF& pos, int rotation, const QString& annotationText ) const;
    QString gridAnnotationString( double value, QgsComposerMap::AnnotationCoordinate coord ) const;
    /**Returns the grid lines with associated coordinate value
        @return 0 in case of success*/
    int xGridLines( QList< QPair< double, QLineF > >& lines ) const;
    /**Returns the grid lines for the y-coordinates. Not vertical in case of rotation
        @return 0 in case of success*/
    int yGridLines( QList< QPair< double, QLineF > >& lines ) const;
    int xGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const;
    int yGridLinesCRSTransform( const QgsRectangle& bbox, const QgsCoordinateTransform& t, QList< QPair< double, QPolygonF > >& lines ) const;
    void drawGridLine( const QLineF& line, QgsRenderContext &context ) const;
    void drawGridLine( const QPolygonF& line, QgsRenderContext &context ) const;
    void sortGridLinesOnBorders( const QList< QPair< double, QLineF > >& hLines, const QList< QPair< double, QLineF > >& vLines,  QMap< double, double >& leftFrameEntries,
                                 QMap< double, double >& rightFrameEntries, QMap< double, double >& topFrameEntries, QMap< double, double >& bottomFrameEntries ) const;
    void drawGridFrameBorder( QPainter* p, const QMap< double, double >& borderPos, QgsComposerMap::Border border ) const;
    /**Returns the item border of a point (in item coordinates)*/
    QgsComposerMap::Border borderForLineCoord( const QPointF& p ) const;
    /**Get parameters for drawing grid in CRS different to map CRS*/
    int crsGridParams( QgsRectangle& crsRect, QgsCoordinateTransform& inverseTransform ) const;
    static QPolygonF trimLineToMap( const QPolygonF& line, const QgsRectangle& rect );
    QPolygonF scalePolygon( const QPolygonF &polygon,  const double scale ) const;

    /**Draws grid if CRS is different to map CRS*/
    void drawGridCRSTransform( QgsRenderContext &context , double dotsPerMM, QList< QPair< double, QLineF > > &horizontalLines,
                               QList< QPair< double, QLineF > > &verticalLines ) const;

    void drawGridNoTransform( QgsRenderContext &context, double dotsPerMM, QList<QPair<double, QLineF> > &horizontalLines, QList<QPair<double, QLineF> > &verticalLines ) const;
    void createDefaultGridLineSymbol();
    void createDefaultGridMarkerSymbol();
    void drawGridMarker( const QPointF &point, QgsRenderContext &context ) const;
};

#endif // QGSCOMPOSERMAPGRID_H
