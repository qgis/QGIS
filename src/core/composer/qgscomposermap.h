/***************************************************************************
                         qgscomposermap.h
                             -------------------
    begin                : January 2005
    copyright            : (C) 2005 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOMPOSERMAP_H
#define QGSCOMPOSERMAP_H

//#include "ui_qgscomposermapbase.h"
#include "qgscomposeritem.h"
#include "qgsrectangle.h"
#include <QFont>
#include <QGraphicsRectItem>

class QgsComposition;
class QgsComposerMapOverviewStack;
class QgsComposerMapOverview;
class QgsComposerMapGridStack;
class QgsComposerMapGrid;
class QgsMapRenderer;
class QgsMapToPixel;
class QDomNode;
class QDomDocument;
class QGraphicsView;
class QPainter;
class QgsFillSymbolV2;
class QgsLineSymbolV2;
class QgsVectorLayer;

/** \ingroup MapComposer
 *  \class QgsComposerMap
 *  \brief Object representing map window.
 */

Q_NOWARN_DEPRECATED_PUSH
class CORE_EXPORT QgsComposerMap : public QgsComposerItem
{
    Q_OBJECT

  public:
    /** Constructor. */
    QgsComposerMap( QgsComposition *composition, int x, int y, int width, int height );
    /** Constructor. Settings are read from project. */
    QgsComposerMap( QgsComposition *composition );
    virtual ~QgsComposerMap();

    /** Return correct graphics item type. */
    virtual int type() const override { return ComposerMap; }

    /** \brief Preview style  */
    enum PreviewMode
    {
      Cache = 0,   // Use raster cache
      Render,      // Render the map
      Rectangle    // Display only rectangle
    };

    //grid enums are moved to QgsComposerMapGrid
    //TODO - remove for QGIS 3.0
    enum GridStyle
    {
      Solid = 0, //solid lines
      Cross, //only draw line crossings
      Markers,
      FrameAnnotationsOnly
    };

    enum GridAnnotationPosition
    {
      InsideMapFrame = 0,
      OutsideMapFrame,
      Disabled
    };

    enum GridAnnotationDirection
    {
      Horizontal = 0,
      Vertical,
      HorizontalAndVertical,
      BoundaryDirection
    };

    enum GridAnnotationFormat
    {
      Decimal = 0,
      DegreeMinute,
      DegreeMinuteSecond,
      DecimalWithSuffix,
      DegreeMinuteNoSuffix,
      DegreeMinutePadded,
      DegreeMinuteSecondNoSuffix,
      DegreeMinuteSecondPadded
    };

    enum GridFrameStyle
    {
      NoGridFrame = 0,
      Zebra, // black/white pattern
      InteriorTicks,
      ExteriorTicks,
      InteriorExteriorTicks,
      LineBorder
    };

    /** Enum for different frame borders*/
    enum Border
    {
      Left,
      Right,
      Bottom,
      Top
    };

    /** Scaling modes used for the serial rendering (atlas)
     */
    enum AtlasScalingMode
    {
      Fixed,      /*!< The current scale of the map is used for each feature of the atlas */
      Predefined, /*!< A scale is chosen from the predefined scales. The smallest scale from
                    the list of scales where the atlas feature is fully visible is chosen.
                    @see QgsAtlasComposition::setPredefinedScales.
                    @note This mode is only valid for polygon or line atlas coverage layers
                */
      Auto        /*!< The extent is adjusted so that each feature is fully visible.
                    A margin is applied around the center @see setAtlasMargin
                    @note This mode is only valid for polygon or line atlas coverage layers*/
    };

    /** \brief Draw to paint device
        @param painter painter
        @param extent map extent
        @param size size in scene coordinates
        @param dpi scene dpi
        @param forceWidthScale force wysiwyg line widths / marker sizes
    */
    void draw( QPainter *painter, const QgsRectangle& extent, const QSizeF& size, double dpi, double* forceWidthScale = 0 );

    /** \brief Reimplementation of QCanvasItem::paint - draw on canvas */
    void paint( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle, QWidget* pWidget ) override;

    /** \brief Create cache image */
    void cache();

    /** Return map settings that would be used for drawing of the map
     *  @note added in 2.6 */
    QgsMapSettings mapSettings( const QgsRectangle& extent, const QSizeF& size, int dpi ) const;

    /** \brief Get identification number*/
    int id() const {return mId;}

    /** True if a draw is already in progress*/
    bool isDrawing() const {return mDrawing;}

    /** Resizes an item in x- and y direction (canvas coordinates)*/
    void resize( double dx, double dy );

    /** Move content of map
       @param dx move in x-direction (item and canvas coordinates)
       @param dy move in y-direction (item and canvas coordinates)*/
    void moveContent( double dx, double dy ) override;

    /** Zoom content of map
     * @param delta value from wheel event that describes direction (positive /negative number)
     * @param x x-position of mouse cursor (in item coordinates)
     * @param y y-position of mouse cursor (in item coordinates)
     * @deprecated use zoomContent( double, QPointF, ZoomMode ) instead
    */
    Q_DECL_DEPRECATED void zoomContent( int delta, double x, double y ) override;

    /** Zoom content of item. Does nothing per default (but implemented in composer map)
     * @param factor zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
     * @param point item point for zoom center
     * @param mode zoom mode
     * @note added in QGIS 2.5
    */
    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom ) override;

    /** Sets new scene rectangle bounds and recalculates hight and extent*/
    void setSceneRect( const QRectF& rectangle ) override;

    /** \brief Scale */
    double scale() const;

    /** Sets new scale and changes only mExtent*/
    void setNewScale( double scaleDenominator, bool forceUpdate = true );

    /** Sets new extent for the map. This method may change the width or height of the map
     * item to ensure that the extent exactly matches the specified extent, with no
     * overlap or margin. This method implicitly alters the map scale.
     * @param extent new extent for the map
     * @see zoomToExtent
    */
    void setNewExtent( const QgsRectangle& extent );

    /** Zooms the map so that the specified extent is fully visible within the map item.
     * This method will not change the width or height of the map, and may result in
     * an overlap or margin from the specified extent. This method implicitly alters the
     * map scale.
     * @param extent new extent for the map
     * @see setNewExtent
     * @note added in QGIS 2.5
    */
    void zoomToExtent( const QgsRectangle& extent );

    /** Sets new Extent for the current atlas preview and changes width, height (and implicitely also scale).
      Atlas preview extents are only temporary, and are regenerated whenever the atlas feature changes
    */
    void setNewAtlasFeatureExtent( const QgsRectangle& extent );

    /** Called when atlas preview is toggled, to force map item to update its extent and redraw
     * @deprecated no longer required
    */
    Q_DECL_DEPRECATED void toggleAtlasPreview() {}

    /** Returns a pointer to the current map extent, which is either the original user specified
     * extent or the temporary atlas-driven feature extent depending on the current atlas state
     * of the composition. Both a const and non-const version are included.
     * @returns pointer to current map extent
     * @see visibleExtentPolygon
    */
    QgsRectangle* currentMapExtent();
    const QgsRectangle* currentMapExtent() const;

    PreviewMode previewMode() const {return mPreviewMode;}
    void setPreviewMode( PreviewMode m );

    /** Getter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas */
    bool keepLayerSet() const {return mKeepLayerSet;}
    /** Setter for flag that determines if the stored layer set should be used or the current layer set of the qgis mapcanvas */
    void setKeepLayerSet( bool enabled ) {mKeepLayerSet = enabled;}

    /** Getter for stored layer set that is used if mKeepLayerSet is true */
    QStringList layerSet() const {return mLayerSet;}
    /** Setter for stored layer set that is used if mKeepLayerSet is true */
    void setLayerSet( const QStringList& layerSet ) {mLayerSet = layerSet;}
    /** Stores the current layer set of the qgis mapcanvas in mLayerSet*/
    void storeCurrentLayerSet();

    /** Getter for flag that determines if current styles of layers should be overridden by previously stored styles. @note added in 2.8 */
    bool keepLayerStyles() const { return mKeepLayerStyles; }
    /** Setter for flag that determines if current styles of layers should be overridden by previously stored styles. @note added in 2.8 */
    void setKeepLayerStyles( bool enabled ) { mKeepLayerStyles = enabled; }

    /** Getter for stored overrides of styles for layers. @note added in 2.8 */
    QMap<QString, QString> layerStyleOverrides() const { return mLayerStyleOverrides; }
    /** Setter for stored overrides of styles for layers. @note added in 2.8 */
    void setLayerStyleOverrides( const QMap<QString, QString>& overrides );
    /** Stores the current layer styles into style overrides. @note added in 2.8 */
    void storeCurrentLayerStyles();

    // Set cache outdated
    void setCacheUpdated( bool u = false );

    QgsRectangle extent() const {return mExtent;}

    //! @deprecated since 2.4 - use mapSettings() - may return 0 if not initialized with QgsMapRenderer
    Q_DECL_DEPRECATED const QgsMapRenderer* mapRenderer() const;

    /** Sets offset values to shift image (useful for live updates when moving item content)*/
    void setOffset( double xOffset, double yOffset );

    /** True if composer map renders a WMS layer*/
    bool containsWMSLayer() const;

    /** True if composer map contains layers with blend modes or flattened layers for vectors */
    bool containsAdvancedEffects() const;

    /** Stores state in Dom node
     * @param elem is Dom element corresponding to 'Composer' tag
     * @param doc Dom document
     */
    bool writeXML( QDomElement& elem, QDomDocument & doc ) const override;

    /** Sets state from Dom document
     * @param itemElem is Dom node corresponding to 'ComposerMap' tag
     * @param doc is Dom document
     */
    bool readXML( const QDomElement& itemElem, const QDomDocument& doc ) override;

    /** Enables a coordinate grid that is shown on top of this composermap.
     * @deprecated use grid()->setEnabled() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridEnabled( bool enabled );

    /**
     * @deprecated use grid()->enabled() or grids() instead
     */
    Q_DECL_DEPRECATED bool gridEnabled() const;

    /** Sets coordinate grid style to solid or cross
     * @deprecated use grid()->setStyle() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridStyle( GridStyle style );

    /**
     * @deprecated use grid()->style() or grids() instead
     */
    Q_DECL_DEPRECATED GridStyle gridStyle() const;

    /** Sets coordinate interval in x-direction for composergrid.
     * @deprecated use grid()->setIntervalX() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridIntervalX( double interval );

    /**
     * @deprecated use grid()->intervalX() or grids() instead
     */
    Q_DECL_DEPRECATED double gridIntervalX() const;

    /** Sets coordinate interval in y-direction for composergrid.
     * @deprecated use grid()->setIntervalY() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridIntervalY( double interval );

    /**
     * @deprecated use grid()->intervalY() or grids() instead
     */
    Q_DECL_DEPRECATED double gridIntervalY() const;

    /** Sets x-coordinate offset for composer grid
     * @deprecated use grid()->setOffsetX() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridOffsetX( double offset );

    /**
     * @deprecated use grid()->offsetX() or grids() instead
     */
    Q_DECL_DEPRECATED double gridOffsetX() const;

    /** Sets y-coordinate offset for composer grid
     * @deprecated use grid()->setOffsetY() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridOffsetY( double offset );

    /**
     * @deprecated use grid()->offsetY() or grids() instead
     */
    Q_DECL_DEPRECATED double gridOffsetY() const;

    /** Sets the pen to draw composer grid
     * @deprecated use grid()->setPenWidth(), grid()->setPenColor() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridPen( const QPen& p );

    /**
     * @deprecated use grid()->pen() or grids() instead
     */
    Q_DECL_DEPRECATED QPen gridPen() const;

    /** Sets width of grid pen
     * @deprecated use grid()->setPenWidth() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridPenWidth( double w );

    /** Sets the color of the grid pen
     * @deprecated use grid()->setPenColor() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridPenColor( const QColor& c );

    /** Sets font for grid annotations
     * @deprecated use grid()->setAnnotationFont() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridAnnotationFont( const QFont& f );

    /**
     * @deprecated use grid()->annotationFont() or grids() instead
     */
    Q_DECL_DEPRECATED QFont gridAnnotationFont() const;

    /** Sets font color for grid annotations
     * @deprecated use grid()->setAnnotationFontColor() or grids() instead
    */
    Q_DECL_DEPRECATED void setAnnotationFontColor( const QColor& c );

    /** Get font color for grid annotations
     * @deprecated use grid()->annotationFontColor() or grids() instead
    */
    Q_DECL_DEPRECATED QColor annotationFontColor() const;

    /** Sets coordinate precision for grid annotations
     * @deprecated use grid()->setAnnotationPrecision or grids() instead
    */
    Q_DECL_DEPRECATED void setGridAnnotationPrecision( int p );

    /**
     * @deprecated use grid()->annotationPrecision() or grids() instead
     */
    Q_DECL_DEPRECATED int gridAnnotationPrecision() const;

    /** Sets flag if grid annotation should be shown
     * @deprecated use grid()->setAnnotationEnabled() or grids() instead
    */
    Q_DECL_DEPRECATED void setShowGridAnnotation( bool show );

    /**
     * @deprecated use grid()->annotationEnabled() or grids() instead
     */
    Q_DECL_DEPRECATED bool showGridAnnotation() const;

    /**
     * @deprecated use grid()->setAnnotationPosition() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridAnnotationPosition( GridAnnotationPosition p, QgsComposerMap::Border border );

    /**
     * @deprecated use grid()->annotationPosition() or grids() instead
     */
    Q_DECL_DEPRECATED GridAnnotationPosition gridAnnotationPosition( QgsComposerMap::Border border ) const;

    /** Sets distance between map frame and annotations
     * @deprecated use grid()->setAnnotationFrameDistance() or grids() instead
    */
    Q_DECL_DEPRECATED void setAnnotationFrameDistance( double d );

    /**
     * @deprecated use grid()->annotationFrameDistance() or grids() instead
     */
    Q_DECL_DEPRECATED double annotationFrameDistance() const;

    /**
     * @deprecated use grid()->setAnnotationDirection() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridAnnotationDirection( GridAnnotationDirection d, QgsComposerMap::Border border );

    /**
     * @deprecated use grid()->annotationDirection() or grids() instead
     */
    Q_DECL_DEPRECATED GridAnnotationDirection gridAnnotationDirection( QgsComposerMap::Border border ) const;

    /**
     * @deprecated use grid()->setAnnotationFormat() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridAnnotationFormat( GridAnnotationFormat f );

    /**
     * @deprecated use grid()->annotationFormat() or grids() instead
     */
    Q_DECL_DEPRECATED GridAnnotationFormat gridAnnotationFormat() const;

    /** Set grid frame style (NoGridFrame or Zebra)
     * @deprecated use grid()->setFrameStyle() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFrameStyle( GridFrameStyle style );

    /**
     * @deprecated use grid()->frameStyle() or grids() instead
     */
    Q_DECL_DEPRECATED GridFrameStyle gridFrameStyle() const;

    /** Set grid frame width
     * @deprecated use grid()->setFrameWidth() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFrameWidth( double w );

    /**
     * @deprecated use grid()->frameWidth() or grids() instead
     */
    Q_DECL_DEPRECATED double gridFrameWidth() const;

    /** Set grid frame pen thickness
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFramePenSize() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFramePenSize( double w );

    /**
     * @deprecated use grid()->framePenSize() or grids() instead
     */
    Q_DECL_DEPRECATED double gridFramePenSize() const;

    /** Sets pen color for grid frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFramePenColor() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFramePenColor( const QColor& c );

    /** Get pen color for grid frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->framePenColor() or grids() instead
    */
    Q_DECL_DEPRECATED QColor gridFramePenColor() const;

    /** Sets first fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFrameFillColor1() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFrameFillColor1( const QColor& c );

    /** Get first fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->frameFillColor1() or grids() instead
    */
    Q_DECL_DEPRECATED QColor gridFrameFillColor1() const;

    /** Sets second fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->setFrameFillColor2() or grids() instead
    */
    Q_DECL_DEPRECATED void setGridFrameFillColor2( const QColor& c );

    /** Get second fill color for grid zebra frame
     * @note: this function was added in version 2.1
     * @deprecated use grid()->frameFillColor2() or grids() instead
    */
    Q_DECL_DEPRECATED QColor gridFrameFillColor2() const;

    /** Sets length of the cross segments (if grid style is cross)
     * @deprecated use grid()->setCrossLength() or grids() instead
    */
    Q_DECL_DEPRECATED void setCrossLength( double l );

    /**
     * @deprecated use grid()->crossLength() or grids() instead
     */
    Q_DECL_DEPRECATED double crossLength();

    /**
     * @deprecated use grid()->setLineSymbol() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridLineSymbol( QgsLineSymbolV2* symbol );

    /**
     * @deprecated use grid()->lineSymbol() or grids() instead
     */
    Q_DECL_DEPRECATED QgsLineSymbolV2* gridLineSymbol();

    /** Returns the grid's blending mode
     * @deprecated use grid()->blendMode() or grids() instead
     */
    Q_DECL_DEPRECATED QPainter::CompositionMode gridBlendMode() const;

    /** Sets the grid's blending mode
     * @deprecated use grid()->setBlendMode() or grids() instead
     */
    Q_DECL_DEPRECATED void setGridBlendMode( QPainter::CompositionMode blendMode );

    /** Returns the map item's grid stack, which is used to control how grids
     * are drawn over the map's contents.
     * @returns pointer to grid stack
     * @see grid()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapGridStack* grids() { return mGridStack; }

    /** Returns the map item's first grid. This is a convenience function.
     * @returns pointer to first grid for map item
     * @see grids()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapGrid* grid();

    /** Returns the map item's overview stack, which is used to control how overviews
     * are drawn over the map's contents.
     * @returns pointer to overview stack
     * @see overview()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapOverviewStack* overviews() { return mOverviewStack; }

    /** Returns the map item's first overview. This is a convenience function.
     * @returns pointer to first overview for map item
     * @see overviews()
     * @note introduced in QGIS 2.5
     */
    QgsComposerMapOverview* overview();

    /** In case of annotations, the bounding rectangle can be larger than the map item rectangle */
    QRectF boundingRect() const override;

    /* reimplement setFrameOutlineWidth, so that updateBoundingRect() is called after setting the frame width */
    virtual void setFrameOutlineWidth( const double outlineWidth ) override;

    /** Sets rotation for the map - this does not affect the composer item shape, only the
      way the map is drawn within the item
     * @deprecated Use setMapRotation( double rotation ) instead
     */
    Q_DECL_DEPRECATED void setRotation( double r ) override;

    /** Returns the rotation used for drawing the map within the composer item
     * @deprecated Use mapRotation() instead
     */
    Q_DECL_DEPRECATED double rotation() const { return mMapRotation;}

    /** Sets rotation for the map - this does not affect the composer item shape, only the
      way the map is drawn within the item
      @note this function was added in version 2.1*/
    void setMapRotation( double r );

    /** Returns the rotation used for drawing the map within the composer item
     * @returns rotation for map
     * @param valueType controls whether the returned value is the user specified rotation,
     * or the current evaluated rotation (which may be affected by data driven rotation
     * settings).
    */
    double mapRotation( QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;

    void updateItem() override;

    /** Sets canvas pointer (necessary to query and draw map canvas items)*/
    void setMapCanvas( QGraphicsView* canvas ) { mMapCanvas = canvas; }

    void setDrawCanvasItems( bool b ) { mDrawCanvasItems = b; }
    bool drawCanvasItems() const { return mDrawCanvasItems; }

    /** Returns the conversion factor map units -> mm*/
    double mapUnitsToMM() const;

    /** Sets overview frame map. -1 disables the overview frame
     * @deprecated use overview()->setFrameMap() or overviews() instead
    */
    Q_DECL_DEPRECATED void setOverviewFrameMap( int mapId );

    /** Returns id of overview frame (or -1 if no overfiew frame)
     * @deprecated use overview()->frameMapId() or overviews() instead
    */
    Q_DECL_DEPRECATED int overviewFrameMapId() const;

    /**
     * @deprecated use overview()->setFrameSymbol() or overviews() instead
    */
    Q_DECL_DEPRECATED void setOverviewFrameMapSymbol( QgsFillSymbolV2* symbol );

    /**
     * @deprecated use overview()->frameSymbol() or overviews() instead
    */
    Q_DECL_DEPRECATED QgsFillSymbolV2* overviewFrameMapSymbol();

    /** Returns the overview's blending mode
     * @deprecated use overview()->blendMode() or overviews() instead
    */
    Q_DECL_DEPRECATED QPainter::CompositionMode overviewBlendMode() const;

    /** Sets the overview's blending mode
     * @deprecated use overview()->setBlendMode() or overviews() instead
     */
    Q_DECL_DEPRECATED void setOverviewBlendMode( QPainter::CompositionMode blendMode );

    /** Returns true if the overview frame is inverted
     * @deprecated use overview()->inverted() or overviews() instead
    */
    Q_DECL_DEPRECATED bool overviewInverted() const;

    /** Sets the overview's inversion mode
     * @deprecated use overview()->setInverted() or overviews() instead
    */
    Q_DECL_DEPRECATED void setOverviewInverted( bool inverted );

    /** Returns true if the extent is forced to center on the overview
     * @deprecated use overview()->centered() or overviews() instead
    */
    Q_DECL_DEPRECATED bool overviewCentered() const;

    /** Set the overview's centering mode
     * @deprecated use overview()->setCentered() or overviews() instead
    */
    Q_DECL_DEPRECATED void setOverviewCentered( bool centered );

    /** Sets mId to a number not yet used in the composition. mId is kept if it is not in use.
        Usually, this function is called before adding the composer map to the composition*/
    void assignFreeId();

    /** Calculates width and hight of the picture (in mm) such that it fits into the item frame with the given rotation
     * @deprecated Use bool QgsComposerItem::imageSizeConsideringRotation( double& width, double& height, double rotation )
     * instead
     */
    Q_DECL_DEPRECATED bool imageSizeConsideringRotation( double& width, double& height ) const;
    /** Calculates corner point after rotation and scaling
     * @deprecated Use QgsComposerItem::cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height, double rotation )
     * instead
     */
    Q_DECL_DEPRECATED bool cornerPointOnRotatedAndScaledRect( double& x, double& y, double width, double height ) const;
    /** Calculates width / height of the bounding box of a rotated rectangle
    * @deprecated Use QgsComposerItem::sizeChangedByRotation( double& width, double& height, double rotation )
    * instead
    */
    Q_DECL_DEPRECATED void sizeChangedByRotation( double& width, double& height );

    /** Returns whether the map extent is set to follow the current atlas feature.
     * @returns true if map will follow the current atlas feature.
     * @see setAtlasDriven
     * @see atlasScalingMode
    */
    bool atlasDriven() const { return mAtlasDriven; }

    /** Sets whether the map extent will follow the current atlas feature.
     * @param enabled set to true if the map extents should be set by the current atlas feature.
     * @see atlasDriven
     * @see setAtlasScalingMode
    */
    void setAtlasDriven( bool enabled );

    /** Returns true if the map uses a fixed scale when in atlas mode
     * @deprecated since 2.4 Use atlasScalingMode() instead
    */
    Q_DECL_DEPRECATED bool atlasFixedScale() const;

    /** Set to true if the map should use a fixed scale when in atlas mode
     * @deprecated since 2.4 Use setAtlasScalingMode() instead
    */
    Q_DECL_DEPRECATED void setAtlasFixedScale( bool fixed );

    /** Returns the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * @returns the current scaling mode
     * @note this parameter is only used if atlasDriven() is true
     * @see setAtlasScalingMode
     * @see atlasDriven
    */
    AtlasScalingMode atlasScalingMode() const { return mAtlasScalingMode; }

    /** Sets the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * @param mode atlas scaling mode to set
     * @note this parameter is only used if atlasDriven() is true
     * @see atlasScalingMode
     * @see atlasDriven
    */
    void setAtlasScalingMode( AtlasScalingMode mode ) { mAtlasScalingMode = mode; }

    /** Returns the margin size (percentage) used when the map is in atlas mode.
     * @param valueType controls whether the returned value is the user specified atlas margin,
     * or the current evaluated atlas margin (which may be affected by data driven atlas margin
     * settings).
     * @returns margin size in percentage to leave around the atlas feature's extent
     * @note this is only used if atlasScalingMode() is Auto.
     * @see atlasScalingMode
     * @see setAtlasMargin
    */
    double atlasMargin( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );

    /** Sets the margin size (percentage) used when the map is in atlas mode.
     * @param margin size in percentage to leave around the atlas feature's extent
     * @note this is only used if atlasScalingMode() is Auto.
     * @see atlasScalingMode
     * @see atlasMargin
    */
    void setAtlasMargin( double margin ) { mAtlasMargin = margin; }

    /** Sets whether updates to the composer map are enabled. */
    void setUpdatesEnabled( bool enabled ) { mUpdatesEnabled = enabled; }

    /** Returns whether updates to the composer map are enabled. */
    bool updatesEnabled() const { return mUpdatesEnabled; }

    /** Get the number of layers that this item requires for exporting as layers
     * @returns 0 if this item is to be placed on the same layer as the previous item,
     * 1 if it should be placed on its own layer, and >1 if it requires multiple export layers
     * @note this method was added in version 2.4
    */
    int numberExportLayers() const override;

    /** Returns a polygon representing the current visible map extent, considering map extents and rotation.
     * If the map rotation is 0, the result is the same as currentMapExtent
     * @returns polygon with the four corner points representing the visible map extent. The points are
     * clockwise, starting at the top-left point
     * @see currentMapExtent
    */
    QPolygonF visibleExtentPolygon() const;

    //overriden to show "Map 1" type names
    virtual QString displayName() const override;

    /** Returns extent that considers rotation and shift with mOffsetX / mOffsetY*/
    QPolygonF transformedMapPolygon() const;

    /** Transforms map coordinates to item coordinates (considering rotation and move offset)*/
    QPointF mapToItemCoords( const QPointF& mapCoords ) const;

    Q_DECL_DEPRECATED void connectMapOverviewSignals();

    /** Calculates the extent to request and the yShift of the top-left point in case of rotation.
     * @note added in 2.6 */
    void requestedExtent( QgsRectangle& extent ) const;

    virtual QgsExpressionContext* createExpressionContext() const override;

  signals:
    void extentChanged();

    /** Is emitted on rotation change to notify north arrow pictures*/
    void mapRotationChanged( double newRotation );

    /** Is emitted when the map has been prepared for atlas rendering, just before actual rendering*/
    void preparedForAtlas();

    /** Emitted when layer style overrides are changed... a means to let
     * associated legend items know they should update
     * @note added in 2.10
     */
    void layerStyleOverridesChanged();

  public slots:

    /** Forces an update of the cached map image*/
    void updateCachedImage();

    /** Updates the cached map image if the map is set to Render mode
     * @see updateCachedImage
    */
    void renderModeUpdateCachedImage();

    /** Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle */
    void updateBoundingRect();

    /** @deprecated use QgsComposerMapOverview::overviewExtentChanged instead*/
    void overviewExtentChanged() {}

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext* context = 0 ) override;

  protected slots:

    /** Called when layers are added or removed from the layer registry. Updates the maps
     * layer set and redraws the map if required.
     * @note added in QGIS 2.9
    */
    void layersChanged();

  private:

    /** Unique identifier*/
    int mId;

    QgsComposerMapGridStack* mGridStack;

    QgsComposerMapOverviewStack* mOverviewStack;

    // Map region in map units realy used for rendering
    // It can be the same as mUserExtent, but it can be bigger in on dimension if mCalculate==Scale,
    // so that full rectangle in paper is used.
    QgsRectangle mExtent;

    // Current temporary map region in map units. This is overwritten when atlas feature changes. It's also
    // used when the user changes the map extent and an atlas preview is enabled. This allows the user
    // to manually tweak each atlas preview page without affecting the actual original map extent.
    QgsRectangle mAtlasFeatureExtent;

    // Cache used in composer preview
    QImage mCacheImage;

    // Is cache up to date
    bool mCacheUpdated;

    /** \brief Preview style  */
    PreviewMode mPreviewMode;

    /** \brief Number of layers when cache was created  */
    int mNumCachedLayers;

    /** \brief set to true if in state of drawing. Concurrent requests to draw method are returned if set to true */
    bool mDrawing;

    /** Offset in x direction for showing map cache image*/
    double mXOffset;
    /** Offset in y direction for showing map cache image*/
    double mYOffset;

    /** Map rotation*/
    double mMapRotation;
    /** Temporary evaluated map rotation. Data defined rotation may mean this value
     * differs from mMapRotation*/
    double mEvaluatedMapRotation;

    /** Flag if layers to be displayed should be read from qgis canvas (true) or from stored list in mLayerSet (false)*/
    bool mKeepLayerSet;

    /** Stored layer list (used if layer live-link mKeepLayerSet is disabled)*/
    QStringList mLayerSet;

    bool mKeepLayerStyles;
    /** Stored style names (value) to be used with particular layer IDs (key) instead of default style */
    QMap<QString, QString> mLayerStyleOverrides;

    /** Whether updates to the map are enabled */
    bool mUpdatesEnabled;

    /** Establishes signal/slot connection for update in case of layer change*/
    void connectUpdateSlot();

    /** Removes layer ids from mLayerSet that are no longer present in the qgis main map*/
    void syncLayerSet();

    /** Returns first map grid or creates an empty one if none*/
    const QgsComposerMapGrid* constFirstMapGrid() const;

    /** Returns first map overview or creates an empty one if none*/
    const QgsComposerMapOverview* constFirstMapOverview() const;

    /** Current bounding rectangle. This is used to check if notification to the graphics scene is necessary*/
    QRectF mCurrentRectangle;
    QGraphicsView* mMapCanvas;
    /** True if annotation items, rubber band, etc. from the main canvas should be displayed*/
    bool mDrawCanvasItems;

    /** Adjusts an extent rectangle to match the provided item width and height, so that extent
     * center of extent remains the same */
    void adjustExtentToItemShape( double itemWidth, double itemHeight, QgsRectangle& extent ) const;

    /** True if map is being controlled by an atlas*/
    bool mAtlasDriven;
    /** Current atlas scaling mode*/
    AtlasScalingMode mAtlasScalingMode;
    /** Margin size for atlas driven extents (percentage of feature size) - when in auto scaling mode*/
    double mAtlasMargin;

    void init();

    /** Resets the item tooltip to reflect current map id*/
    void updateToolTip();

    /** Returns a list of the layers to render for this map item*/
    QStringList layersToRender() const;

    /** Returns current layer style overrides for this map item*/
    QMap<QString, QString> layerStyleOverridesToRender() const;

    /** Returns extent that considers mOffsetX / mOffsetY (during content move)*/
    QgsRectangle transformedExtent() const;

    /** MapPolygon variant using a given extent */
    void mapPolygon( const QgsRectangle& extent, QPolygonF& poly ) const;

    /** Scales a composer map shift (in MM) and rotates it by mRotation
        @param xShift in: shift in x direction (in item units), out: xShift in map units
        @param yShift in: shift in y direction (in item units), out: yShift in map units*/
    void transformShift( double& xShift, double& yShift ) const;

    void drawCanvasItems( QPainter* painter, const QStyleOptionGraphicsItem* itemStyle );
    void drawCanvasItem( QGraphicsItem* item, QPainter* painter, const QStyleOptionGraphicsItem* itemStyle );
    QPointF composerMapPosForItem( const QGraphicsItem* item ) const;

    enum PartType
    {
      Background,
      Layer,
      Grid,
      OverviewMapExtent,
      Frame,
      SelectionBoxes
    };

    /** Test if a part of the copmosermap needs to be drawn, considering mCurrentExportLayer*/
    bool shouldDrawPart( PartType part ) const;

    /** Refresh the map's extents, considering data defined extent, scale and rotation
     * @param context expression context for evaluating data defined map parameters
     * @note this method was added in version 2.5
     */
    void refreshMapExtents( const QgsExpressionContext* context = 0 );

    friend class QgsComposerMapOverview; //to access mXOffset, mYOffset
    friend class TestQgsComposerMap;
};
Q_NOWARN_DEPRECATED_POP

#endif

