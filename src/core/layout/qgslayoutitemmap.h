/***************************************************************************
                              qgslayoutitemmap.h
                             -------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTITEMMAP_H
#define QGSLAYOUTITEMMAP_H

#include "qgis_core.h"
#include "qgslayoutitem.h"
#include "qgslayoutitemregistry.h"
#include "qgsmaplayerref.h"
#include "qgsmaprenderercustompainterjob.h"
#include "qgslayoutitemmapgrid.h"
#include "qgslayoutitemmapoverview.h"

class QgsAnnotation;

/**
 * \ingroup core
 * \class QgsLayoutItemMap
 * \brief Layout graphical items for displaying a map.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutItemMap : public QgsLayoutItem
{

    Q_OBJECT

  public:

    /**
     * Scaling modes used for the serial rendering (atlas)
     */
    enum AtlasScalingMode
    {
      Fixed,      //!< The current scale of the map is used for each feature of the atlas

      /**
       * A scale is chosen from the predefined scales. The smallest scale from
       * the list of scales where the atlas feature is fully visible is chosen.
       * \see QgsAtlasComposition::setPredefinedScales.
       * \note This mode is only valid for polygon or line atlas coverage layers
       */
      Predefined,

      /**
       * The extent is adjusted so that each feature is fully visible.
       * A margin is applied around the center \see setAtlasMargin
       * \note This mode is only valid for polygon or line atlas coverage layers
      */
      Auto
    };

    /**
     * Various flags that affect drawing of map items.
     * \since QGIS 3.6
     */
    enum MapItemFlag
    {
      ShowPartialLabels  = 1 << 0,  //!< Whether to draw labels which are partially outside of the map view
    };
    Q_DECLARE_FLAGS( MapItemFlags, MapItemFlag )

    /**
     * Constructor for QgsLayoutItemMap, with the specified parent \a layout.
     */
    explicit QgsLayoutItemMap( QgsLayout *layout );
    ~QgsLayoutItemMap() override;

    int type() const override;
    QIcon icon() const override;
    QgsLayoutItem::Flags itemFlags() const override;

    /**
     * Returns the map item's flags, which control how the map content is drawn.
     * \see setMapFlags()
     * \since QGIS 3.6
     */
    QgsLayoutItemMap::MapItemFlags mapFlags() const;

    /**
     * Sets the map item's \a flags, which control how the map content is drawn.
     * \see mapFlags()
     * \since QGIS 3.6
     */
    void setMapFlags( QgsLayoutItemMap::MapItemFlags flags );

    /**
     * Sets the map id() to a number not yet used in the layout. The existing id() is kept if it is not in use.
    */
    void assignFreeId();

    //overridden to show "Map 1" type names
    QString displayName() const override;

    /**
     * Returns a new map item for the specified \a layout.
     *
     * The caller takes responsibility for deleting the returned object.
     */
    static QgsLayoutItemMap *create( QgsLayout *layout ) SIP_FACTORY;

    // for now, map items behave a bit differently and don't implement draw. TODO - see if we can avoid this
    void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget ) override;
    int numberExportLayers() const override;
    void setFrameStrokeWidth( QgsLayoutMeasurement width ) override;

    /**
     * Returns the map scale.
     * The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     * \see setScale()
     */
    double scale() const;

    /**
     * Sets new map \a scale and changes only the map extent.
     *
     * The \a scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
     *
     * \see scale()
     */
    void setScale( double scale, bool forceUpdate = true );

    /**
     * Sets a new \a extent for the map. This method may change the width or height of the map
     * item to ensure that the extent exactly matches the specified extent, with no
     * overlap or margin. This method implicitly alters the map scale.
     * \see zoomToExtent()
     * \see extentChanged()
     */
    void setExtent( const QgsRectangle &extent );

    /**
     * Zooms the map so that the specified \a extent is fully visible within the map item.
     * This method will not change the width or height of the map, and may result in
     * an overlap or margin from the specified extent. This method implicitly alters the
     * map scale.
     * \see setExtent()
     */
    void zoomToExtent( const QgsRectangle &extent );

    /**
     * Returns the current map extent.
     * \see visibleExtentPolygon()
     * \see extentChanged()
     */
    QgsRectangle extent() const;


    /**
     * Returns a polygon representing the current visible map extent, considering map extents and rotation.
     * If the map rotation is 0, the result is the same as currentMapExtent
     * \returns polygon with the four corner points representing the visible map extent. The points are
     * clockwise, starting at the top-left point
     * \see extent()
     */
    QPolygonF visibleExtentPolygon() const;

    /**
     * Returns coordinate reference system used for rendering the map.
     * This will match the presetCrs() if that is set, or if a preset
     * CRS is not set then the map's CRS will follow the composition's
     * project's CRS.
     * \see presetCrs()
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem crs() const;

    /**
     * Returns the map's preset coordinate reference system. If set, this
     * CRS will be used to render the map regardless of any project CRS
     * setting. If the returned CRS is not valid then the project CRS
     * will be used to render the map.
     * \see crs()
     * \see setCrs()
     */
    QgsCoordinateReferenceSystem presetCrs() const { return mCrs; }

    /**
     * Sets the map's preset \a crs (coordinate reference system). If a valid CRS is
     * set, this CRS will be used to render the map regardless of any project CRS
     * setting. If the CRS is not valid then the project CRS will be used to render the map.
     * \see crs()
     * \see presetCrs()
     */
    void setCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Returns whether a stored layer set should be used
     * or the current layer set from the project associated with the layout. This is just a GUI flag,
     * and itself does not change which layers are rendered in the map.
     * Instead, use setLayers() to control which layers are rendered.
     * \see setKeepLayerSet()
     * \see layers()
     */
    bool keepLayerSet() const { return mKeepLayerSet; }

    /**
     * Sets whether the stored layer set should be used
     * or the current layer set of the associated project. This is just a GUI flag,
     * and itself does not change which layers are rendered in the map.
     * Instead, use setLayers() to control which layers are rendered.
     * \see keepLayerSet()
     * \see layers()
     */
    void setKeepLayerSet( bool enabled ) { mKeepLayerSet = enabled; }

    /**
     * Returns the stored layer set. If empty, the current project layers will
     * be used instead.
     * \see setLayers()
     * \see keepLayerSet()
     */
    QList<QgsMapLayer *> layers() const;

    /**
     * Sets the stored \a layers set. If empty, the current project layers will
     * be used instead.
     * \see layers()
     * \see keepLayerSet()
     */
    void setLayers( const QList<QgsMapLayer *> &layers );

    /**
     * Returns whether current styles of layers should be overridden by previously stored styles.
     * \see setKeepLayerStyles()
     */
    bool keepLayerStyles() const { return mKeepLayerStyles; }

    /**
     * Sets whether current styles of layers should be overridden by previously stored styles.
     * \see keepLayerStyles()
     */
    void setKeepLayerStyles( bool enabled ) { mKeepLayerStyles = enabled; }

    /**
     * Returns stored overrides of styles for layers.
     * \see setLayerStyleOverrides()
     */
    QMap<QString, QString> layerStyleOverrides() const { return mLayerStyleOverrides; }

    /**
     * Sets the stored overrides of styles for layers.
     * \see layerStyleOverrides()
     */
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );

    /**
     * Stores the current project layer styles into style overrides.
     */
    void storeCurrentLayerStyles();

    /**
     * Returns whether the map should follow a map theme. If true, the layers and layer styles
     * will be used from given preset name (configured with setFollowVisibilityPresetName() method).
     * This means when preset's settings are changed, the new settings are automatically
     * picked up next time when rendering, without having to explicitly update them.
     * At most one of the flags keepLayerSet() and followVisibilityPreset() should be enabled
     * at any time since they are alternative approaches - if both are enabled,
     * following map theme has higher priority. If neither is enabled (or if preset name is not set),
     * map will use the same configuration as the map canvas uses.
     */
    bool followVisibilityPreset() const { return mFollowVisibilityPreset; }

    /**
     * Sets whether the map should follow a map theme. See followVisibilityPreset() for more details.
     */
    void setFollowVisibilityPreset( bool follow ) { mFollowVisibilityPreset = follow; }

    /**
     * Preset name that decides which layers and layer styles are used for map rendering. It is only
     * used when followVisibilityPreset() returns true.
     * \see setFollowVisibilityPresetName()
     */
    QString followVisibilityPresetName() const { return mFollowVisibilityPresetName; }

    /**
     * Sets preset name for map rendering. See followVisibilityPresetName() for more details.
     * \see followVisibilityPresetName()
    */
    void setFollowVisibilityPresetName( const QString &name ) { mFollowVisibilityPresetName = name; }

    void moveContent( double dx, double dy ) override;
    void setMoveContentPreviewOffset( double dx, double dy ) override;

    void zoomContent( double factor, QPointF point ) override;


    //! Returns true if the map contains a WMS layer.
    bool containsWmsLayer() const;

    bool requiresRasterization() const override;
    bool containsAdvancedEffects() const override;

    /**
     * Sets the \a rotation for the map - this does not affect the layout item shape, only the
     * way the map is drawn within the item. Rotation is in degrees, clockwise.
     * \see mapRotation()
     * \see mapRotationChanged()
     */
    void setMapRotation( double rotation );

    /**
     * Returns the rotation used for drawing the map within the layout item, in degrees clockwise.
     * \param valueType controls whether the returned value is the user specified rotation,
     * or the current evaluated rotation (which may be affected by data driven rotation
     * settings).
     * \see setMapRotation()
     * \see mapRotationChanged()
     */
    double mapRotation( QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue ) const;

    /**
     * Sets whether annotations are drawn within the map.
     * \see drawAnnotations()
     */
    void setDrawAnnotations( bool draw ) { mDrawAnnotations = draw; }

    /**
     * Returns whether annotations are drawn within the map.
     * \see setDrawAnnotations()
     */
    bool drawAnnotations() const { return mDrawAnnotations; }


    /**
     * Returns whether the map extent is set to follow the current atlas feature.
     * \returns true if map will follow the current atlas feature.
     * \see setAtlasDriven
     * \see atlasScalingMode
     */
    bool atlasDriven() const { return mAtlasDriven; }

    /**
     * Sets whether the map extent will follow the current atlas feature.
     * \param enabled set to true if the map extents should be set by the current atlas feature.
     * \see atlasDriven
     * \see setAtlasScalingMode
     */
    void setAtlasDriven( bool enabled );

    /**
     * Returns the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * \returns the current scaling mode
     * \note this parameter is only used if atlasDriven() is true
     * \see setAtlasScalingMode
     * \see atlasDriven
     */
    AtlasScalingMode atlasScalingMode() const { return mAtlasScalingMode; }

    /**
     * Sets the current atlas scaling mode. This controls how the map's extents
     * are calculated for the current atlas feature when an atlas composition
     * is enabled.
     * \param mode atlas scaling mode to set
     * \note this parameter is only used if atlasDriven() is true
     * \see atlasScalingMode
     * \see atlasDriven
     */
    void setAtlasScalingMode( AtlasScalingMode mode ) { mAtlasScalingMode = mode; }

    /**
     * Returns the margin size (percentage) used when the map is in atlas mode.
     * \param valueType controls whether the returned value is the user specified atlas margin,
     * or the current evaluated atlas margin (which may be affected by data driven atlas margin
     * settings).
     * \returns margin size in percentage to leave around the atlas feature's extent
     * \note this is only used if atlasScalingMode() is Auto.
     * \see atlasScalingMode
     * \see setAtlasMargin
     */
    double atlasMargin( QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue );

    /**
     * Sets the margin size (percentage) used when the map is in atlas mode.
     * \param margin size in percentage to leave around the atlas feature's extent
     * \note this is only used if atlasScalingMode() is Auto.
     * \see atlasScalingMode
     * \see atlasMargin
     */
    void setAtlasMargin( double margin ) { mAtlasMargin = margin; }

    /**
     * Returns the map item's grid stack, which is used to control how grids
     * are drawn over the map's contents.
     * \see grid()
     */
    QgsLayoutItemMapGridStack *grids() { return mGridStack.get(); }

    /**
     * Returns the map item's first grid. This is a convenience function.
     * \see grids()
     */
    QgsLayoutItemMapGrid *grid();

    /**
     * Returns the map item's overview stack, which is used to control how overviews
     * are drawn over the map's contents.
     * \returns pointer to overview stack
     * \see overview()
     */
    QgsLayoutItemMapOverviewStack *overviews() { return mOverviewStack.get(); }

    /**
     * Returns the map item's first overview. This is a convenience function.
     * \returns pointer to first overview for map item
     * \see overviews()
     */
    QgsLayoutItemMapOverview *overview();

    /**
     * Returns the margin from the map edges in which no labels may be placed.
     *
     * If the margin is 0 then labels can be placed right up to the edge (and possibly overlapping the edge)
     * of the map.
     *
     * \see setLabelMargin()
     *
     * \since QGIS 3.6
     */
    QgsLayoutMeasurement labelMargin() const;

    /**
     * Sets the \a margin from the map edges in which no labels may be placed.
     *
     * If the margin is 0 then labels can be placed right up to the edge (and possibly overlapping the edge)
     * of the map.
     *
     * \see labelMargin()
     *
     * \since QGIS 3.6
     */
    void setLabelMargin( const QgsLayoutMeasurement &margin );

    QgsExpressionContext createExpressionContext() const override;

    /**
     * Returns the conversion factor from map units to layout units.
     * This is calculated using the width of the map item and the width of the
     * current visible map extent.
     */
    double mapUnitsToLayoutUnits() const;

    /**
     * Returns map settings that will be used for drawing of the map.
     *
     * If \a includeLayerSettings is true, than settings specifically relating to map layers and map layer styles
     * will be calculated. This can be expensive to calculate, so if they are not required in the map settings
     * (e.g. for map settings which are used for scale related calculations only) then \a includeLayerSettings should be false.
     */
    QgsMapSettings mapSettings( const QgsRectangle &extent, QSizeF size, double dpi, bool includeLayerSettings ) const;

    void finalizeRestoreFromXml() override;

    /**
     * Returns a list of the layers which will be rendered within this map item, considering
     * any locked layers, linked map theme, and data defined settings.
     */
    QList<QgsMapLayer *> layersToRender( const QgsExpressionContext *context = nullptr ) const;

    /**
     * Sets the specified layout \a item as a "label blocking item" for this map.
     *
     * Items which are marked as label blocking items prevent any map labels from being placed
     * in the area of the map item covered by the \a item.
     *
     * \see removeLabelBlockingItem()
     * \see isLabelBlockingItem()
     *
     * \since QGIS 3.6
     */
    void addLabelBlockingItem( QgsLayoutItem *item );

    /**
     * Removes the specified layout \a item from the map's "label blocking items".
     *
     * Items which are marked as label blocking items prevent any map labels from being placed
     * in the area of the map item covered by the item.
     *
     * \see addLabelBlockingItem()
     * \see isLabelBlockingItem()
     *
     * \since QGIS 3.6
     */
    void removeLabelBlockingItem( QgsLayoutItem *item );

    /**
     * Returns true if the specified \a item is a "label blocking item".
     *
     * Items which are marked as label blocking items prevent any map labels from being placed
     * in the area of the map item covered by the item.
     *
     * \see addLabelBlockingItem()
     * \see removeLabelBlockingItem()
     *
     * \since QGIS 3.6
     */
    bool isLabelBlockingItem( QgsLayoutItem *item ) const;

  protected:

    void draw( QgsLayoutItemRenderContext &context ) override;
    bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const override;
    bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context ) override;

    //! True if a draw is already in progress
    bool isDrawing() const {return mDrawing;}

    // In case of annotations, the bounding rectangle can be larger than the map item rectangle
    QRectF boundingRect() const override;

    //! Returns extent that considers rotation and shift with mOffsetX / mOffsetY
    QPolygonF transformedMapPolygon() const;

    //! Transforms map coordinates to item coordinates (considering rotation and move offset)
    QPointF mapToItemCoords( QPointF mapCoords ) const;

    /**
     * Calculates the extent to request and the yShift of the top-left point in case of rotation.
     */
    QgsRectangle requestedExtent() const;

  signals:

    /**
     * Is emitted when the map's extent changes.
     * \see setExtent()
     * \see extent()
     */
    void extentChanged();

    /**
     * Is emitted when the map's rotation changes.
     * \see setMapRotation()
     * \see mapRotation()
     */
    void mapRotationChanged( double newRotation );

    //! Is emitted when the map has been prepared for atlas rendering, just before actual rendering
    void preparedForAtlas();

    /**
     * Emitted when layer style overrides are changed... a means to let
     * associated legend items know they should update
     */
    void layerStyleOverridesChanged();

  public slots:

    void refresh() override;

    void invalidateCache() override;

    //! Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
    void updateBoundingRect();

    void refreshDataDefinedProperty( QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties ) override;

  private slots:
    void layersAboutToBeRemoved( const QList<QgsMapLayer *> &layers );

    void painterJobFinished();

    void shapeChanged();

    void mapThemeChanged( const QString &theme );

    //! Create cache image
    void recreateCachedImageInBackground();

  private:

    QgsLayoutItemMap::MapItemFlags mMapFlags = nullptr;

    //! Unique identifier
    int mMapId = 1;

    std::unique_ptr< QgsLayoutItemMapGridStack > mGridStack;
    std::unique_ptr< QgsLayoutItemMapOverviewStack > mOverviewStack;

    // Map region in map units really used for rendering
    // It can be the same as mUserExtent, but it can be bigger in on dimension if mCalculate==Scale,
    // so that full rectangle in paper is used.
    QgsRectangle mExtent;

    //! Map CRS
    QgsCoordinateReferenceSystem mCrs;

    // Current temporary map region in map units. This is overwritten when atlas feature changes. It's also
    // used when the user changes the map extent and an atlas preview is enabled. This allows the user
    // to manually tweak each atlas preview page without affecting the actual original map extent.
    QgsRectangle mAtlasFeatureExtent;

    // We have two images used for rendering/storing cached map images.
    // the first (mCacheFinalImage) is used ONLY for storing the most recent completed map render. It's always
    // used when drawing map item previews. The second (mCacheRenderingImage) is used temporarily while
    // rendering a new preview image in the background. If (and only if) the background render completes, then
    // mCacheRenderingImage is pushed into mCacheFinalImage, and used from then on when drawing the item preview.
    // This ensures that something is always shown in the map item, even while refreshing the preview image in the
    // background
    std::unique_ptr< QImage > mCacheFinalImage;
    std::unique_ptr< QImage > mCacheRenderingImage;
    bool mUpdatesEnabled = true;

    //! True if cached map image must be recreated
    bool mCacheInvalidated = true;

    //! \brief Number of layers when cache was created
    int mNumCachedLayers;

    // Set to true if in state of drawing. Concurrent requests to draw method are returned if set to true
    bool mDrawing = false;

    QTimer *mBackgroundUpdateTimer = nullptr;
    double mPreviewScaleFactor = 0;

    bool mDrawingPreview = false;

    //! Offset in x direction for showing map cache image
    double mXOffset = 0.0;
    //! Offset in y direction for showing map cache image
    double mYOffset = 0.0;

    double mLastRenderedImageOffsetX = 0.0;
    double mLastRenderedImageOffsetY = 0.0;

    //! Map rotation
    double mMapRotation = 0;

    /**
     * Temporary evaluated map rotation. Data defined rotation may mean this value
     * differs from mMapRotation*/
    double mEvaluatedMapRotation = 0;

    //! Flag if layers to be displayed should be read from qgis canvas (true) or from stored list in mLayerSet (false)
    bool mKeepLayerSet = false;

    //! Stored layer list (used if layer live-link mKeepLayerSet is disabled)
    QList< QgsMapLayerRef > mLayers;

    bool mKeepLayerStyles = false;
    //! Stored style names (value) to be used with particular layer IDs (key) instead of default style
    QMap<QString, QString> mLayerStyleOverrides;

    //! Empty if no cached style overrides stored
    mutable QString mCachedLayerStyleOverridesPresetName;
    //! Cached style overrides, used to avoid frequent expensive lookups of the preset style override
    mutable QMap<QString, QString> mCachedPresetLayerStyleOverrides;

    /**
     * Whether layers and styles should be used from a preset (preset name is stored
     * in mVisibilityPresetName and may be overridden by data-defined expression).
     * This flag has higher priority than mKeepLayerSet. */
    bool mFollowVisibilityPreset = false;

    /**
     * Map theme name to be used for map's layers and styles in case mFollowVisibilityPreset
     *  is true. May be overridden by data-defined expression. */
    QString mFollowVisibilityPresetName;

    /**
     * \brief Draw to paint device
     *  \param painter painter
     *  \param extent map extent
     *  \param size size in scene coordinates
     *  \param dpi scene dpi
     */
    void drawMap( QPainter *painter, const QgsRectangle &extent, QSizeF size, double dpi );

    //! Establishes signal/slot connection for update in case of layer change
    void connectUpdateSlot();

    //! Removes layer ids from mLayerSet that are no longer present in the qgis main map
    void syncLayerSet();

    //! Returns first map grid or creates an empty one if none
    const QgsLayoutItemMapGrid *constFirstMapGrid() const;

    //! Returns first map overview or creates an empty one if none
    const QgsLayoutItemMapOverview *constFirstMapOverview() const;

    /**
     * Creates a transform from layout coordinates to map coordinates.
     */
    QTransform layoutToMapCoordsTransform() const;

    /**
     * Creates a list of label blocking regions for the map, which correspond to the
     * map areas covered by other layout items marked as label blockers for this map.
     */
    QList< QgsLabelBlockingRegion > createLabelBlockingRegions( const QgsMapSettings &mapSettings ) const;

    //! Current bounding rectangle. This is used to check if notification to the graphics scene is necessary
    QRectF mCurrentRectangle;
    //! True if annotation items, rubber band, etc. from the main canvas should be displayed
    bool mDrawAnnotations = true;

    //! True if map is being controlled by an atlas
    bool mAtlasDriven = false;
    //! Current atlas scaling mode
    AtlasScalingMode mAtlasScalingMode = Auto;
    //! Margin size for atlas driven extents (percentage of feature size) - when in auto scaling mode
    double mAtlasMargin = 0.10;

    std::unique_ptr< QPainter > mPainter;
    std::unique_ptr< QgsMapRendererCustomPainterJob > mPainterJob;
    bool mPainterCancelWait = false;

    QgsLayoutMeasurement mLabelMargin{ 0 };
    QgsLayoutMeasurement mEvaluatedLabelMargin{ 0 };

    QStringList mBlockingLabelItemUuids;
    QList< QPointer< QgsLayoutItem > > mBlockingLabelItems;

    void init();

    //! Resets the item tooltip to reflect current map id
    void updateToolTip();

    //! Returns current layer style overrides for this map item
    QMap<QString, QString> layerStyleOverridesToRender( const QgsExpressionContext &context ) const;

    //! Returns extent that considers mOffsetX / mOffsetY (during content move)
    QgsRectangle transformedExtent() const;

    //! MapPolygon variant using a given extent
    void mapPolygon( const QgsRectangle &extent, QPolygonF &poly ) const;

    /**
     * Scales a layout map shift (in layout units) and rotates it by mRotation
     * \param xShift in: shift in x direction (in item units), out: xShift in map units
     * \param yShift in: shift in y direction (in item units), out: yShift in map units
    */
    void transformShift( double &xShift, double &yShift ) const;

    void drawAnnotations( QPainter *painter );
    void drawAnnotation( const QgsAnnotation *item, QgsRenderContext &context );
    QPointF layoutMapPosForItem( const QgsAnnotation *item ) const;

    void drawMapFrame( QPainter *p );
    void drawMapBackground( QPainter *p );

    enum PartType
    {
      Background,
      Layer,
      Grid,
      OverviewMapExtent,
      Frame,
      SelectionBoxes
    };

    //! Test if a part of the item needs to be drawn, considering the context's current export layer
    bool shouldDrawPart( PartType part ) const;

    /**
     * Refresh the map's extents, considering data defined extent, scale and rotation
     * \param context expression context for evaluating data defined map parameters
     */
    void refreshMapExtents( const QgsExpressionContext *context = nullptr );

    void refreshLabelMargin( bool updateItem );

    void updateAtlasFeature();

    QgsRectangle computeAtlasRectangle();

    friend class QgsLayoutItemMapGrid;
    friend class QgsLayoutItemMapOverview;
    friend class QgsLayoutItemLegend;
    friend class TestQgsLayoutMap;
    friend class QgsCompositionConverter;

};

Q_DECLARE_OPERATORS_FOR_FLAGS( QgsLayoutItemMap::MapItemFlags )

#endif //QGSLAYOUTITEMMAP_H
