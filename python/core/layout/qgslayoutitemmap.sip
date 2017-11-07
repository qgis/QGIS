/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsLayoutItemMap : QgsLayoutItem
{
%Docstring
 Layout graphical items for displaying a map.
.. versionadded:: 3.0
%End

%TypeHeaderCode
#include "qgslayoutitemmap.h"
%End
  public:

    enum AtlasScalingMode
    {
      Fixed,

      Predefined,

      Auto
    };

    explicit QgsLayoutItemMap( QgsLayout *layout );
%Docstring
 Constructor for QgsLayoutItemMap, with the specified parent ``layout``.
%End
    ~QgsLayoutItemMap();

    virtual int type() const;

    virtual QString stringType() const;


    void assignFreeId();
%Docstring
 Sets the map id() to a number not yet used in the layout. The existing id() is kept if it is not in use.
%End

    virtual QString displayName() const;

    static QgsLayoutItemMap *create( QgsLayout *layout ) /Factory/;
%Docstring
 Returns a new map item for the specified ``layout``.

 The caller takes responsibility for deleting the returned object.
 :rtype: QgsLayoutItemMap
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

    virtual int numberExportLayers() const;

    virtual void setFrameStrokeWidth( const QgsLayoutMeasurement &width );



    double scale() const;
%Docstring
 Returns the map scale.
 The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.
.. seealso:: setScale()
 :rtype: float
%End

    void setScale( double scale, bool forceUpdate = true );
%Docstring
 Sets new map ``scale`` and changes only the map extent.

 The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: scale()
%End

    void setExtent( const QgsRectangle &extent );
%Docstring
 Sets a new ``extent`` for the map. This method may change the width or height of the map
 item to ensure that the extent exactly matches the specified extent, with no
 overlap or margin. This method implicitly alters the map scale.
.. seealso:: zoomToExtent()
.. seealso:: extentChanged()
%End

    void zoomToExtent( const QgsRectangle &extent );
%Docstring
 Zooms the map so that the specified ``extent`` is fully visible within the map item.
 This method will not change the width or height of the map, and may result in
 an overlap or margin from the specified extent. This method implicitly alters the
 map scale.
.. seealso:: setExtent()
%End

    QgsRectangle extent() const;
%Docstring
 Returns the current map extent.
.. seealso:: visibleExtentPolygon()
.. seealso:: extentChanged()
 :rtype: QgsRectangle
%End


    QPolygonF visibleExtentPolygon() const;
%Docstring
 Returns a polygon representing the current visible map extent, considering map extents and rotation.
 If the map rotation is 0, the result is the same as currentMapExtent
 :return: polygon with the four corner points representing the visible map extent. The points are
 clockwise, starting at the top-left point
.. seealso:: extent()
 :rtype: QPolygonF
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
 Returns coordinate reference system used for rendering the map.
 This will match the presetCrs() if that is set, or if a preset
 CRS is not set then the map's CRS will follow the composition's
 project's CRS.
.. seealso:: presetCrs()
.. seealso:: setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    QgsCoordinateReferenceSystem presetCrs() const;
%Docstring
 Returns the map's preset coordinate reference system. If set, this
 CRS will be used to render the map regardless of any project CRS
 setting. If the returned CRS is not valid then the project CRS
 will be used to render the map.
.. seealso:: crs()
.. seealso:: setCrs()
 :rtype: QgsCoordinateReferenceSystem
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
 Sets the map's preset ``crs`` (coordinate reference system). If a valid CRS is
 set, this CRS will be used to render the map regardless of any project CRS
 setting. If the CRS is not valid then the project CRS will be used to render the map.
.. seealso:: crs()
.. seealso:: presetCrs()
%End

    bool keepLayerSet() const;
%Docstring
 Returns whether a stored layer set should be used
 or the current layer set from the project associated with the layout. This is just a GUI flag,
 and itself does not change which layers are rendered in the map.
 Instead, use setLayers() to control which layers are rendered.
.. seealso:: setKeepLayerSet()
.. seealso:: layers()
 :rtype: bool
%End

    void setKeepLayerSet( bool enabled );
%Docstring
 Sets whether the stored layer set should be used
 or the current layer set of the associated project. This is just a GUI flag,
 and itself does not change which layers are rendered in the map.
 Instead, use setLayers() to control which layers are rendered.
.. seealso:: keepLayerSet()
.. seealso:: layers()
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
 Returns the stored layer set. If empty, the current project layers will
 be used instead.
.. seealso:: setLayers()
.. seealso:: keepLayerSet()
 :rtype: list of QgsMapLayer
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
 Sets the stored ``layers`` set. If empty, the current project layers will
 be used instead.
.. seealso:: layers()
.. seealso:: keepLayerSet()
%End

    bool keepLayerStyles() const;
%Docstring
 Returns whether current styles of layers should be overridden by previously stored styles.
.. seealso:: setKeepLayerStyles()
 :rtype: bool
%End

    void setKeepLayerStyles( bool enabled );
%Docstring
 Sets whether current styles of layers should be overridden by previously stored styles.
.. seealso:: keepLayerStyles()
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
 Returns stored overrides of styles for layers.
.. seealso:: setLayerStyleOverrides()
 :rtype: QMap<str, QString>
%End

    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
 Sets the stored overrides of styles for layers.
.. seealso:: layerStyleOverrides()
%End

    void storeCurrentLayerStyles();
%Docstring
 Stores the current project layer styles into style overrides.
%End

    bool followVisibilityPreset() const;
%Docstring
 Returns whether the map should follow a map theme. If true, the layers and layer styles
 will be used from given preset name (configured with setFollowVisibilityPresetName() method).
 This means when preset's settings are changed, the new settings are automatically
 picked up next time when rendering, without having to explicitly update them.
 At most one of the flags keepLayerSet() and followVisibilityPreset() should be enabled
 at any time since they are alternative approaches - if both are enabled,
 following map theme has higher priority. If neither is enabled (or if preset name is not set),
 map will use the same configuration as the map canvas uses.
 :rtype: bool
%End

    void setFollowVisibilityPreset( bool follow );
%Docstring
 Sets whether the map should follow a map theme. See followVisibilityPreset() for more details.
%End

    QString followVisibilityPresetName() const;
%Docstring
 Preset name that decides which layers and layer styles are used for map rendering. It is only
 used when followVisibilityPreset() returns true.
.. seealso:: setFollowVisibilityPresetName()
 :rtype: str
%End

    void setFollowVisibilityPresetName( const QString &name );
%Docstring
 Sets preset name for map rendering. See followVisibilityPresetName() for more details.
.. seealso:: followVisibilityPresetName()
%End

    virtual void moveContent( double dx, double dy );

    virtual void setMoveContentPreviewOffset( double dx, double dy );


    virtual void zoomContent( double factor, QPointF point );



    bool containsWmsLayer() const;
%Docstring
Returns true if the map contains a WMS layer.
 :rtype: bool
%End

    bool containsAdvancedEffects() const;
%Docstring
Returns true if the map contains layers with blend modes or flattened layers for vectors
 :rtype: bool
%End

    void setMapRotation( double rotation );
%Docstring
 Sets the ``rotation`` for the map - this does not affect the composer item shape, only the
 way the map is drawn within the item. Rotation is in degrees, clockwise.
.. seealso:: mapRotation()
.. seealso:: mapRotationChanged()
%End

    double mapRotation( QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue ) const;
%Docstring
 Returns the rotation used for drawing the map within the composer item, in degrees clockwise.
 \param valueType controls whether the returned value is the user specified rotation,
 or the current evaluated rotation (which may be affected by data driven rotation
 settings).
.. seealso:: setMapRotation()
.. seealso:: mapRotationChanged()
 :rtype: float
%End

    void setDrawAnnotations( bool draw );
%Docstring
 Sets whether annotations are drawn within the composer map.
.. seealso:: drawAnnotations()
%End

    bool drawAnnotations() const;
%Docstring
 Returns whether annotations are drawn within the composer map.
.. seealso:: setDrawAnnotations()
 :rtype: bool
%End


    bool atlasDriven() const;
%Docstring
 Returns whether the map extent is set to follow the current atlas feature.
 :return: true if map will follow the current atlas feature.
.. seealso:: setAtlasDriven
.. seealso:: atlasScalingMode
 :rtype: bool
%End

    void setAtlasDriven( bool enabled );
%Docstring
 Sets whether the map extent will follow the current atlas feature.
 \param enabled set to true if the map extents should be set by the current atlas feature.
.. seealso:: atlasDriven
.. seealso:: setAtlasScalingMode
%End

    AtlasScalingMode atlasScalingMode() const;
%Docstring
 Returns the current atlas scaling mode. This controls how the map's extents
 are calculated for the current atlas feature when an atlas composition
 is enabled.
 :return: the current scaling mode
.. note::

   this parameter is only used if atlasDriven() is true
.. seealso:: setAtlasScalingMode
.. seealso:: atlasDriven
 :rtype: AtlasScalingMode
%End

    void setAtlasScalingMode( AtlasScalingMode mode );
%Docstring
 Sets the current atlas scaling mode. This controls how the map's extents
 are calculated for the current atlas feature when an atlas composition
 is enabled.
 \param mode atlas scaling mode to set
.. note::

   this parameter is only used if atlasDriven() is true
.. seealso:: atlasScalingMode
.. seealso:: atlasDriven
%End

    double atlasMargin( const QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue );
%Docstring
 Returns the margin size (percentage) used when the map is in atlas mode.
 \param valueType controls whether the returned value is the user specified atlas margin,
 or the current evaluated atlas margin (which may be affected by data driven atlas margin
 settings).
 :return: margin size in percentage to leave around the atlas feature's extent
.. note::

   this is only used if atlasScalingMode() is Auto.
.. seealso:: atlasScalingMode
.. seealso:: setAtlasMargin
 :rtype: float
%End

    void setAtlasMargin( double margin );
%Docstring
 Sets the margin size (percentage) used when the map is in atlas mode.
 \param margin size in percentage to leave around the atlas feature's extent
.. note::

   this is only used if atlasScalingMode() is Auto.
.. seealso:: atlasScalingMode
.. seealso:: atlasMargin
%End

    QgsLayoutItemMapGridStack *grids();
%Docstring
 Returns the map item's grid stack, which is used to control how grids
 are drawn over the map's contents.
.. seealso:: grid()
 :rtype: QgsLayoutItemMapGridStack
%End

    QgsLayoutItemMapGrid *grid();
%Docstring
 Returns the map item's first grid. This is a convenience function.
.. seealso:: grids()
 :rtype: QgsLayoutItemMapGrid
%End

    QgsLayoutItemMapOverviewStack *overviews();
%Docstring
 Returns the map item's overview stack, which is used to control how overviews
 are drawn over the map's contents.
 :return: pointer to overview stack
.. seealso:: overview()
 :rtype: QgsLayoutItemMapOverviewStack
%End

    QgsLayoutItemMapOverview *overview();
%Docstring
 Returns the map item's first overview. This is a convenience function.
 :return: pointer to first overview for map item
.. seealso:: overviews()
 :rtype: QgsLayoutItemMapOverview
%End

    virtual QgsExpressionContext createExpressionContext() const;


    double mapUnitsToLayoutUnits() const;
%Docstring
 Returns the conversion factor from map units to layout units.
 This is calculated using the width of the map item and the width of the
 current visible map extent.
 :rtype: float
%End

    QgsMapSettings mapSettings( const QgsRectangle &extent, QSizeF size, int dpi ) const;
%Docstring
 Return map settings that will be used for drawing of the map.
 :rtype: QgsMapSettings
%End

  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


    bool isDrawing() const;
%Docstring
True if a draw is already in progress
 :rtype: bool
%End


    virtual QRectF boundingRect() const;


    QPolygonF transformedMapPolygon() const;
%Docstring
Returns extent that considers rotation and shift with mOffsetX / mOffsetY
 :rtype: QPolygonF
%End

    QPointF mapToItemCoords( QPointF mapCoords ) const;
%Docstring
Transforms map coordinates to item coordinates (considering rotation and move offset)
 :rtype: QPointF
%End

    QgsRectangle requestedExtent() const;
%Docstring
 Calculates the extent to request and the yShift of the top-left point in case of rotation.
 :rtype: QgsRectangle
%End

  signals:

    void extentChanged();
%Docstring
 Is emitted when the map's extent changes.
.. seealso:: setExtent()
.. seealso:: extent()
%End

    void mapRotationChanged( double newRotation );
%Docstring
 Is emitted when the map's rotation changes.
.. seealso:: setMapRotation()
.. seealso:: mapRotation()
%End

    void preparedForAtlas();
%Docstring
Is emitted when the map has been prepared for atlas rendering, just before actual rendering
%End

    void layerStyleOverridesChanged();
%Docstring
 Emitted when layer style overrides are changed... a means to let
 associated legend items know they should update
%End

  public slots:

    virtual void invalidateCache();


    void updateBoundingRect();
%Docstring
Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
%End

    virtual void refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property = QgsLayoutObject::AllProperties );


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/layout/qgslayoutitemmap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
