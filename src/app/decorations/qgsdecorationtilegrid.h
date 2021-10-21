/***************************************************************************
 qgsdecorationtilegrid.h
 -----------------------
 Date: 24-Nov-2021
 Copyright: (C) 2021 by Jochen Topf
 Email: jochen@topf.org
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

class QgsLineSymbol;
class QgsMarkerSymbol;

#include <QFont>
#include "qgis_app.h"

class APP_EXPORT QgsDecorationTileGrid : public QgsDecorationItem
{

    Q_OBJECT

  public:

    QgsDecorationTileGrid( QObject *parent = nullptr );
    ~ QgsDecorationTileGrid() override;

    enum GridStyle
    {
      Line = 0,
      Marker
    };

    enum DynamicOrStaticGrid
    {
      Dynamic = 0,
      Static
    };

    enum class GridAnnotationStyle
    {
      None = 0,
      Center,
      Border
    };

    /**
     * Returns the text format used for coordinate annotations.
     * \see setTextFormat()
     */
    QgsTextFormat textFormat() const { return mTextFormat; }

    /**
     * Sets the coordinate annotation text \a format.
     * \see textFormat()
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

    //! Sets coordinate grid style.
    void setGridStyle( GridStyle style ) noexcept { mGridStyle = style; }
    GridStyle gridStyle() const noexcept { return mGridStyle; }

    //! Sets dynamic or static grid
    void setDynamicOrStaticGrid( DynamicOrStaticGrid val ) noexcept { mDynamicOrStaticGrid = val; }
    DynamicOrStaticGrid dynamicOrStaticGrid() const noexcept { return mDynamicOrStaticGrid; }

    //! Sets zoom level (for static grid)
    void setZoomLevel( int zoom ) noexcept { mZoomLevel = zoom; }
    int zoomLevel() const noexcept { return mZoomLevel; }

    //! Sets zoom factor (for dynamic grid)
    void setZoomFactor( double value ) noexcept { mZoomFactor = value; }
    double zoomFactor() const noexcept { return mZoomFactor; }

    //! Sets font for grid annotations
    void setGridAnnotationFont( const QFont &f ) { mGridAnnotationFont = f; }
    QFont gridAnnotationFont() const noexcept { return mGridAnnotationFont; }

    //! Sets style of grid annotation
    void setGridAnnotationStyle( GridAnnotationStyle style ) noexcept { mGridAnnotationStyle = style; }
    GridAnnotationStyle gridAnnotationStyle() const noexcept { return mGridAnnotationStyle; }

    bool showGridAnnotation() const noexcept { return mGridAnnotationStyle != GridAnnotationStyle::None; }

    //! Sets symbol that is used to draw grid lines. Takes ownership
    void setLineSymbol( QgsLineSymbol *symbol );
    const QgsLineSymbol *lineSymbol() const noexcept { return mLineSymbol.get(); }

    //! Sets symbol that is used to draw markers. Takes ownership
    void setMarkerSymbol( QgsMarkerSymbol *symbol );
    const QgsMarkerSymbol *markerSymbol() const noexcept { return mMarkerSymbol.get(); }

  public slots:
    //! Sets values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! this does the meaty bit of the work
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;
    //! Show the dialog box
    void run() override;

    //! Called when the map CRS changed to possiby disable decoration
    void mapCrsChanged();

  private:

    //! Line or Symbol
    GridStyle mGridStyle;

    //! Dynamic (resizes automatically) or static (one zoom level) grid
    DynamicOrStaticGrid mDynamicOrStaticGrid;

    //! Style in which the grid coordinates should be shown
    GridAnnotationStyle mGridAnnotationStyle = GridAnnotationStyle::None;

    //! Zoom level (for static grid)
    int mZoomLevel;
    //! Zoom factor (for dynamic grid)
    double mZoomFactor;

    //! Font for tile annotation
    QFont mGridAnnotationFont;

    std::unique_ptr<QgsLineSymbol> mLineSymbol{};
    std::unique_ptr<QgsMarkerSymbol> mMarkerSymbol{};

    int zoomFromVisibleExtent( const QgsMapSettings &mapSettings ) const;
    void drawTileCoordinates( QgsRenderContext &context, int zoom ) const;

    QgsTextFormat mTextFormat;
};

#endif
