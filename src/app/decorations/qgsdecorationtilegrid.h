/***************************************************************************
                         qgsdecorationtilegrid.h
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
#ifndef QGSDECORATIONTILEGRID_H
#define QGSDECORATIONTILEGRID_H

#include "qgsdecorationitem.h"
#include "qgstextformat.h"

class QPainter;
class QgsLineSymbol;
class QgsMarkerSymbol;

#include <QColor>
#include <QPen>
#include <QFont>
#include "qgis_app.h"

class APP_EXPORT QgsDecorationTileGrid: public QgsDecorationItem
{

    Q_OBJECT

  public:

    QgsDecorationTileGrid( QObject *parent = nullptr );
    ~ QgsDecorationTileGrid() override;

    enum GridStyle
    {
      Line = 0, // lines
      Marker //markers
    };

    enum DynamicOrStaticGrid
    {
      Dynamic = 0,
      Static
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

    //! Sets dynamic or static grid
    void setDynamicOrStaticGrid( DynamicOrStaticGrid val ) { mDynamicOrStaticGrid = val; }
    DynamicOrStaticGrid dynamicOrStaticGrid() const { return mDynamicOrStaticGrid; }

    //! Sets zoom level (for static grid)
    void setZoomLevel( int zoom ) { mZoomLevel = zoom; }
    int zoomLevel() const { return mZoomLevel; }

    //! Sets zoom factor (for dynamic grid)
    void setZoomFactor( double value ) { mZoomFactor = value; }
    double zoomFactor() const { return mZoomFactor; }

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

    //! Sets flag if grid annotation should be shown
    void setShowGridAnnotation( bool show ) { mShowGridAnnotation = show; }
    bool showGridAnnotation() const { return mShowGridAnnotation; }

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

    //! Line or Symbol
    GridStyle mGridStyle;

    //! Dynamic (resizes automatically) or static (one zoom level) grid
    DynamicOrStaticGrid mDynamicOrStaticGrid;
    //! Zoom level (for static grid)
    int mZoomLevel = 3;
    //! Zoom factor (for dynamic grid)
    double mZoomFactor = 1.0;
    //! Grid line pen
    QPen mGridPen;
    //! Font for tile annotation
    QFont mGridAnnotationFont;
    //! True if tile coordinates should be drawn
    bool mShowGridAnnotation;

    std::unique_ptr<QgsLineSymbol> mLineSymbol{};
    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol{};

    QgsUnitTypes::DistanceUnit mMapUnits;

    int zoomFromVisibleExtent( const QgsMapSettings &mapSettings ) const;
    void drawTileCoordinates( QgsRenderContext &context, int zoom );

    QgsTextFormat mTextFormat;
};

#endif
