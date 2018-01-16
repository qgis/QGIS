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

    virtual QIcon icon() const;


    void assignFreeId();
%Docstring
Sets the map id() to a number not yet used in the layout. The existing id() is kept if it is not in use.
%End

    virtual QString displayName() const;


    static QgsLayoutItemMap *create( QgsLayout *layout ) /Factory/;
%Docstring
Returns a new map item for the specified ``layout``.

The caller takes responsibility for deleting the returned object.
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );

    virtual int numberExportLayers() const;

    virtual void setFrameStrokeWidth( const QgsLayoutMeasurement &width );


    double scale() const;
%Docstring
Returns the map scale.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`setScale`
%End

    void setScale( double scale, bool forceUpdate = true );
%Docstring
Sets new map ``scale`` and changes only the map extent.

The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`scale`
%End

    void setExtent( const QgsRectangle &extent );
%Docstring
Sets a new ``extent`` for the map. This method may change the width or height of the map
item to ensure that the extent exactly matches the specified extent, with no
overlap or margin. This method implicitly alters the map scale.

.. seealso:: :py:func:`zoomToExtent`

.. seealso:: :py:func:`extentChanged`
%End

    void zoomToExtent( const QgsRectangle &extent );
%Docstring
Zooms the map so that the specified ``extent`` is fully visible within the map item.
This method will not change the width or height of the map, and may result in
an overlap or margin from the specified extent. This method implicitly alters the
map scale.

.. seealso:: :py:func:`setExtent`
%End

    QgsRectangle extent() const;
%Docstring
Returns the current map extent.

.. seealso:: :py:func:`visibleExtentPolygon`

.. seealso:: :py:func:`extentChanged`
%End


    QPolygonF visibleExtentPolygon() const;
%Docstring
Returns a polygon representing the current visible map extent, considering map extents and rotation.
If the map rotation is 0, the result is the same as currentMapExtent

:return: polygon with the four corner points representing the visible map extent. The points are
clockwise, starting at the top-left point

.. seealso:: :py:func:`extent`
%End

    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns coordinate reference system used for rendering the map.
This will match the presetCrs() if that is set, or if a preset
CRS is not set then the map's CRS will follow the composition's
project's CRS.

.. seealso:: :py:func:`presetCrs`

.. seealso:: :py:func:`setCrs`
%End

    QgsCoordinateReferenceSystem presetCrs() const;
%Docstring
Returns the map's preset coordinate reference system. If set, this
CRS will be used to render the map regardless of any project CRS
setting. If the returned CRS is not valid then the project CRS
will be used to render the map.

.. seealso:: :py:func:`crs`

.. seealso:: :py:func:`setCrs`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the map's preset ``crs`` (coordinate reference system). If a valid CRS is
set, this CRS will be used to render the map regardless of any project CRS
setting. If the CRS is not valid then the project CRS will be used to render the map.

.. seealso:: :py:func:`crs`

.. seealso:: :py:func:`presetCrs`
%End

    bool keepLayerSet() const;
%Docstring
Returns whether a stored layer set should be used
or the current layer set from the project associated with the layout. This is just a GUI flag,
and itself does not change which layers are rendered in the map.
Instead, use setLayers() to control which layers are rendered.

.. seealso:: :py:func:`setKeepLayerSet`

.. seealso:: :py:func:`layers`
%End

    void setKeepLayerSet( bool enabled );
%Docstring
Sets whether the stored layer set should be used
or the current layer set of the associated project. This is just a GUI flag,
and itself does not change which layers are rendered in the map.
Instead, use setLayers() to control which layers are rendered.

.. seealso:: :py:func:`keepLayerSet`

.. seealso:: :py:func:`layers`
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
Returns the stored layer set. If empty, the current project layers will
be used instead.

.. seealso:: :py:func:`setLayers`

.. seealso:: :py:func:`keepLayerSet`
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Sets the stored ``layers`` set. If empty, the current project layers will
be used instead.

.. seealso:: :py:func:`layers`

.. seealso:: :py:func:`keepLayerSet`
%End

    bool keepLayerStyles() const;
%Docstring
Returns whether current styles of layers should be overridden by previously stored styles.

.. seealso:: :py:func:`setKeepLayerStyles`
%End

    void setKeepLayerStyles( bool enabled );
%Docstring
Sets whether current styles of layers should be overridden by previously stored styles.

.. seealso:: :py:func:`keepLayerStyles`
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring
Returns stored overrides of styles for layers.

.. seealso:: :py:func:`setLayerStyleOverrides`
%End

    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring
Sets the stored overrides of styles for layers.

.. seealso:: :py:func:`layerStyleOverrides`
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
%End

    void setFollowVisibilityPreset( bool follow );
%Docstring
Sets whether the map should follow a map theme. See followVisibilityPreset() for more details.
%End

    QString followVisibilityPresetName() const;
%Docstring
Preset name that decides which layers and layer styles are used for map rendering. It is only
used when followVisibilityPreset() returns true.

.. seealso:: :py:func:`setFollowVisibilityPresetName`
%End

    void setFollowVisibilityPresetName( const QString &name );
%Docstring
Sets preset name for map rendering. See followVisibilityPresetName() for more details.

.. seealso:: :py:func:`followVisibilityPresetName`
%End

    virtual void moveContent( double dx, double dy );

    virtual void setMoveContentPreviewOffset( double dx, double dy );


    virtual void zoomContent( double factor, QPointF point );



    bool containsWmsLayer() const;
%Docstring
Returns true if the map contains a WMS layer.
%End

    virtual bool requiresRasterization() const;

    virtual bool containsAdvancedEffects() const;


    void setMapRotation( double rotation );
%Docstring
Sets the ``rotation`` for the map - this does not affect the composer item shape, only the
way the map is drawn within the item. Rotation is in degrees, clockwise.

.. seealso:: :py:func:`mapRotation`

.. seealso:: :py:func:`mapRotationChanged`
%End

    double mapRotation( QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue ) const;
%Docstring
Returns the rotation used for drawing the map within the composer item, in degrees clockwise.

:param valueType: controls whether the returned value is the user specified rotation,
or the current evaluated rotation (which may be affected by data driven rotation
settings).

.. seealso:: :py:func:`setMapRotation`

.. seealso:: :py:func:`mapRotationChanged`
%End

    void setDrawAnnotations( bool draw );
%Docstring
Sets whether annotations are drawn within the composer map.

.. seealso:: :py:func:`drawAnnotations`
%End

    bool drawAnnotations() const;
%Docstring
Returns whether annotations are drawn within the composer map.

.. seealso:: :py:func:`setDrawAnnotations`
%End


    bool atlasDriven() const;
%Docstring
Returns whether the map extent is set to follow the current atlas feature.

:return: true if map will follow the current atlas feature.

.. seealso:: :py:func:`setAtlasDriven`

.. seealso:: :py:func:`atlasScalingMode`
%End

    void setAtlasDriven( bool enabled );
%Docstring
Sets whether the map extent will follow the current atlas feature.

:param enabled: set to true if the map extents should be set by the current atlas feature.

.. seealso:: :py:func:`atlasDriven`

.. seealso:: :py:func:`setAtlasScalingMode`
%End

    AtlasScalingMode atlasScalingMode() const;
%Docstring
Returns the current atlas scaling mode. This controls how the map's extents
are calculated for the current atlas feature when an atlas composition
is enabled.

:return: the current scaling mode

.. note::

   this parameter is only used if atlasDriven() is true

.. seealso:: :py:func:`setAtlasScalingMode`

.. seealso:: :py:func:`atlasDriven`
%End

    void setAtlasScalingMode( AtlasScalingMode mode );
%Docstring
Sets the current atlas scaling mode. This controls how the map's extents
are calculated for the current atlas feature when an atlas composition
is enabled.

:param mode: atlas scaling mode to set

.. note::

   this parameter is only used if atlasDriven() is true

.. seealso:: :py:func:`atlasScalingMode`

.. seealso:: :py:func:`atlasDriven`
%End

    double atlasMargin( const QgsLayoutObject::PropertyValueType valueType = QgsLayoutObject::EvaluatedValue );
%Docstring
Returns the margin size (percentage) used when the map is in atlas mode.

:param valueType: controls whether the returned value is the user specified atlas margin,
or the current evaluated atlas margin (which may be affected by data driven atlas margin
settings).

:return: margin size in percentage to leave around the atlas feature's extent

.. note::

   this is only used if atlasScalingMode() is Auto.

.. seealso:: :py:func:`atlasScalingMode`

.. seealso:: :py:func:`setAtlasMargin`
%End

    void setAtlasMargin( double margin );
%Docstring
Sets the margin size (percentage) used when the map is in atlas mode.

:param margin: size in percentage to leave around the atlas feature's extent

.. note::

   this is only used if atlasScalingMode() is Auto.

.. seealso:: :py:func:`atlasScalingMode`

.. seealso:: :py:func:`atlasMargin`
%End

    QgsLayoutItemMapGridStack *grids();
%Docstring
Returns the map item's grid stack, which is used to control how grids
are drawn over the map's contents.

.. seealso:: :py:func:`grid`
%End

    QgsLayoutItemMapGrid *grid();
%Docstring
Returns the map item's first grid. This is a convenience function.

.. seealso:: :py:func:`grids`
%End

    QgsLayoutItemMapOverviewStack *overviews();
%Docstring
Returns the map item's overview stack, which is used to control how overviews
are drawn over the map's contents.

:return: pointer to overview stack

.. seealso:: :py:func:`overview`
%End

    QgsLayoutItemMapOverview *overview();
%Docstring
Returns the map item's first overview. This is a convenience function.

:return: pointer to first overview for map item

.. seealso:: :py:func:`overviews`
%End

    virtual QgsExpressionContext createExpressionContext() const;


    double mapUnitsToLayoutUnits() const;
%Docstring
Returns the conversion factor from map units to layout units.
This is calculated using the width of the map item and the width of the
current visible map extent.
%End

    QgsMapSettings mapSettings( const QgsRectangle &extent, QSizeF size, double dpi ) const;
%Docstring
Return map settings that will be used for drawing of the map.
%End

    virtual void finalizeRestoreFromXml();


  protected:

    virtual void draw( QgsRenderContext &context, const QStyleOptionGraphicsItem *itemStyle = 0 );

    virtual bool writePropertiesToElement( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const;

    virtual bool readPropertiesFromElement( const QDomElement &element, const QDomDocument &document, const QgsReadWriteContext &context );


    bool isDrawing() const;
%Docstring
True if a draw is already in progress
%End

    virtual QRectF boundingRect() const;


    QPolygonF transformedMapPolygon() const;
%Docstring
Returns extent that considers rotation and shift with mOffsetX / mOffsetY
%End

    QPointF mapToItemCoords( QPointF mapCoords ) const;
%Docstring
Transforms map coordinates to item coordinates (considering rotation and move offset)
%End

    QgsRectangle requestedExtent() const;
%Docstring
Calculates the extent to request and the yShift of the top-left point in case of rotation.
%End

  signals:

    void extentChanged();
%Docstring
Is emitted when the map's extent changes.

.. seealso:: :py:func:`setExtent`

.. seealso:: :py:func:`extent`
%End

    void mapRotationChanged( double newRotation );
%Docstring
Is emitted when the map's rotation changes.

.. seealso:: :py:func:`setMapRotation`

.. seealso:: :py:func:`mapRotation`
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

    virtual void refresh();


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
