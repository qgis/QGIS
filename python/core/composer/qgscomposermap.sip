/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsComposerMap : QgsComposerItem
{
%Docstring
 Object representing map window.
%End

%TypeHeaderCode
#include "qgscomposermap.h"
%End
  public:
    QgsComposerMap( QgsComposition *composition /TransferThis/, int x, int y, int width, int height );
%Docstring
Constructor.
%End
    QgsComposerMap( QgsComposition *composition /TransferThis/ );
%Docstring
Constructor. Settings are read from project.
%End
    ~QgsComposerMap();

    virtual int type() const;
%Docstring
Return correct graphics item type.

:rtype: int
%End

    enum AtlasScalingMode
    {
      Fixed,

      Predefined,

      Auto
    };

    void draw( QPainter *painter, const QgsRectangle &extent, QSizeF size, double dpi, double *forceWidthScale = 0 );
%Docstring
 Draw to paint device
:param painter: painter
:param extent: map extent
:param size: size in scene coordinates
:param dpi: scene dpi
:param forceWidthScale: force wysiwyg line widths / marker sizes
%End

    virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget );


    QgsMapSettings mapSettings( const QgsRectangle &extent, QSizeF size, int dpi ) const;
%Docstring
Return map settings that would be used for drawing of the map

:rtype: QgsMapSettings

.. versionadded:: 2.6
%End

    int id() const;
%Docstring
 Get identification number

:rtype: int
%End

    bool isDrawing() const;
%Docstring
True if a draw is already in progress

:rtype: bool
%End

    void resize( double dx, double dy );
%Docstring
Resizes an item in x- and y direction (canvas coordinates)
%End

    virtual void moveContent( double dx, double dy );

%Docstring
Move content of map
:param dx: move in x-direction (item and canvas coordinates)
:param dy: move in y-direction (item and canvas coordinates)
%End

    virtual void zoomContent( const double factor, const QPointF point, const ZoomMode mode = QgsComposerItem::Zoom );

%Docstring
Zoom content of item. Does nothing per default (but implemented in composer map)
:param factor: zoom factor, where > 1 results in a zoom in and < 1 results in a zoom out
:param point: item point for zoom center
:param mode: zoom mode

.. versionadded:: 2.5
%End

    virtual void setSceneRect( const QRectF &rectangle );

%Docstring
Sets new scene rectangle bounds and recalculates hight and extent
%End

    double scale() const;
%Docstring
Returns the map scale.
The scale value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

:rtype: float

.. seealso:: :py:func:`setNewScale()`
%End

    void setNewScale( double scaleDenominator, bool forceUpdate = true );
%Docstring
Sets new map ``scale`` and changes only the map extent.
The ``scale`` value indicates the scale denominator, e.g. 1000.0 for a 1:1000 map.

.. seealso:: :py:func:`scale()`
%End

    void setNewExtent( const QgsRectangle &extent );
%Docstring
Sets new extent for the map. This method may change the width or height of the map
item to ensure that the extent exactly matches the specified extent, with no
overlap or margin. This method implicitly alters the map scale.
:param extent: new extent for the map

.. seealso:: :py:func:`zoomToExtent`
%End

    void zoomToExtent( const QgsRectangle &extent );
%Docstring
Zooms the map so that the specified extent is fully visible within the map item.
This method will not change the width or height of the map, and may result in
an overlap or margin from the specified extent. This method implicitly alters the
map scale.
:param extent: new extent for the map

.. seealso:: :py:func:`setNewExtent`

.. versionadded:: 2.5
%End

    void setNewAtlasFeatureExtent( const QgsRectangle &extent );
%Docstring
Sets new Extent for the current atlas preview and changes width, height (and implicitly also scale).
Atlas preview extents are only temporary, and are regenerated whenever the atlas feature changes
%End

    const QgsRectangle *currentMapExtent() const;
%Docstring
Returns a pointer to the current map extent, which is either the original user specified
extent or the temporary atlas-driven feature extent depending on the current atlas state
of the composition. Both a const and non-const version are included.

:return: pointer to current map extent
:rtype: QgsRectangle


.. seealso:: :py:func:`visibleExtentPolygon`
%End


    QgsCoordinateReferenceSystem crs() const;
%Docstring
Returns coordinate reference system used for rendering the map.
This will match the presetCrs() if that is set, or if a preset
CRS is not set then the map's CRS will follow the composition's
project's CRS.

:rtype: QgsCoordinateReferenceSystem

.. versionadded:: 3.0

.. seealso:: :py:func:`presetCrs()`

.. seealso:: :py:func:`setCrs()`
%End

    QgsCoordinateReferenceSystem presetCrs() const;
%Docstring
Returns the map's preset coordinate reference system. If set, this
CRS will be used to render the map regardless of any project CRS
setting. If the returned CRS is not valid then the project CRS
will be used to render the map.

:rtype: QgsCoordinateReferenceSystem

.. versionadded:: 3.0

.. seealso:: :py:func:`crs()`

.. seealso:: :py:func:`setCrs()`
%End

    void setCrs( const QgsCoordinateReferenceSystem &crs );
%Docstring
Sets the map's preset coordinate reference system. If a valid CRS is
set, this CRS will be used to render the map regardless of any project CRS
setting. If the CRS is not valid then the project CRS will be used to render the map.

.. seealso:: :py:func:`crs()`

.. seealso:: :py:func:`presetCrs()`

.. versionadded:: 3.0
%End

    bool keepLayerSet() const;
%Docstring
Getter for flag that determines if a stored layer set should be used
or the current layer set of the QGIS map canvas. This is just a GUI flag,
and itself does not change which layers are rendered in the map.
Instead, use setLayers() to control which layers are rendered.

:rtype: bool

.. seealso:: :py:func:`setKeepLayerSet()`

.. seealso:: :py:func:`layers()`
%End

    void setKeepLayerSet( bool enabled );
%Docstring
Setter for flag that determines if the stored layer set should be used
or the current layer set of the QGIS map canvas. This is just a GUI flag,
and itself does not change which layers are rendered in the map.
Instead, use setLayers() to control which layers are rendered.

.. seealso:: :py:func:`keepLayerSet()`

.. seealso:: :py:func:`layers()`
%End

    QList<QgsMapLayer *> layers() const;
%Docstring
Getter for stored layer set. If empty, the current canvas layers will
be used instead.

:rtype: list of QgsMapLayer

.. seealso:: :py:func:`setLayers()`

.. seealso:: :py:func:`keepLayerSet()`
%End

    void setLayers( const QList<QgsMapLayer *> &layers );
%Docstring
Setter for stored layer set. If empty, the current canvas layers will
be used instead.

.. seealso:: :py:func:`layers()`

.. seealso:: :py:func:`keepLayerSet()`
%End

    bool keepLayerStyles() const;
%Docstring

:rtype: bool

.. versionadded:: 2.8
%End
    void setKeepLayerStyles( bool enabled );
%Docstring

.. versionadded:: 2.8
%End

    QMap<QString, QString> layerStyleOverrides() const;
%Docstring

:rtype: QMap<str, QString>

.. versionadded:: 2.8
%End
    void setLayerStyleOverrides( const QMap<QString, QString> &overrides );
%Docstring

.. versionadded:: 2.8
%End
    void storeCurrentLayerStyles();
%Docstring

.. versionadded:: 2.8
%End

    bool followVisibilityPreset() const;
%Docstring
Whether the map should follow a map theme. If true, the layers and layer styles
will be used from given preset name (configured with setFollowVisibilityPresetName() method).
This means when preset's settings are changed, the new settings are automatically
picked up next time when rendering, without having to explicitly update them.
At most one of the flags keepLayerSet() and followVisibilityPreset() should be enabled
at any time since they are alternative approaches - if both are enabled,
following map theme has higher priority. If neither is enabled (or if preset name is not set),
map will use the same configuration as the map canvas uses.

:rtype: bool

.. versionadded:: 2.16
%End

    void setFollowVisibilityPreset( bool follow );
%Docstring
Sets whether the map should follow a map theme. See followVisibilityPreset() for more details.

.. versionadded:: 2.16
%End

    QString followVisibilityPresetName() const;
%Docstring
Preset name that decides which layers and layer styles are used for map rendering. It is only
used when followVisibilityPreset() returns true.

:rtype: str

.. versionadded:: 2.16
%End

    void setFollowVisibilityPresetName( const QString &name );
%Docstring
Sets preset name for map rendering. See followVisibilityPresetName() for more details.

.. versionadded:: 2.16
%End

    QgsRectangle extent() const;
%Docstring

:rtype: QgsRectangle
%End

    void setOffset( double xOffset, double yOffset );
%Docstring
Sets offset values to shift image (useful for live updates when moving item content)
%End

    bool containsWmsLayer() const;
%Docstring
True if composer map renders a WMS layer

:rtype: bool
%End

    bool containsAdvancedEffects() const;
%Docstring
True if composer map contains layers with blend modes or flattened layers for vectors

:rtype: bool
%End

    virtual bool writeXml( QDomElement &elem, QDomDocument &doc ) const;

%Docstring
Stores state in Dom node
:param elem: is Dom element corresponding to 'Composer' tag
:param doc: Dom document

:rtype: bool
%End

    virtual bool readXml( const QDomElement &itemElem, const QDomDocument &doc );

%Docstring
Sets state from Dom document
:param itemElem: is Dom node corresponding to 'ComposerMap' tag
:param doc: is Dom document

:rtype: bool
%End

    QgsComposerMapGridStack *grids();
%Docstring
Returns the map item's grid stack, which is used to control how grids
are drawn over the map's contents.

:return: pointer to grid stack
:rtype: QgsComposerMapGridStack


.. seealso:: :py:func:`grid()`

.. versionadded:: 2.5
%End

    QgsComposerMapGrid *grid();
%Docstring
Returns the map item's first grid. This is a convenience function.

:return: pointer to first grid for map item
:rtype: QgsComposerMapGrid


.. seealso:: :py:func:`grids()`

.. versionadded:: 2.5
%End

    QgsComposerMapOverviewStack *overviews();
%Docstring
Returns the map item's overview stack, which is used to control how overviews
are drawn over the map's contents.

:return: pointer to overview stack
:rtype: QgsComposerMapOverviewStack


.. seealso:: :py:func:`overview()`

.. versionadded:: 2.5
%End

    QgsComposerMapOverview *overview();
%Docstring
Returns the map item's first overview. This is a convenience function.

:return: pointer to first overview for map item
:rtype: QgsComposerMapOverview


.. seealso:: :py:func:`overviews()`

.. versionadded:: 2.5
%End

    virtual QRectF boundingRect() const;

%Docstring
In case of annotations, the bounding rectangle can be larger than the map item rectangle

:rtype: QRectF
%End

    virtual void setFrameStrokeWidth( const double strokeWidth );

%Docstring
reimplement setFrameStrokeWidth, so that updateBoundingRect() is called after setting the frame width */
%End

    void setMapRotation( double rotation );
%Docstring
Sets the ``rotation`` for the map - this does not affect the composer item shape, only the
way the map is drawn within the item. Rotation is in degrees, clockwise.

.. versionadded:: 2.1

.. seealso:: :py:func:`mapRotation()`
%End

    double mapRotation( QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue ) const;
%Docstring
Returns the rotation used for drawing the map within the composer item, in degrees clockwise.
:param valueType: controls whether the returned value is the user specified rotation,
or the current evaluated rotation (which may be affected by data driven rotation
settings).

:rtype: float

.. seealso:: :py:func:`setMapRotation()`
%End

    void setDrawAnnotations( bool draw );
%Docstring
Sets whether annotations are drawn within the composer map.

.. seealso:: :py:func:`drawAnnotations()`
%End

    bool drawAnnotations() const;
%Docstring
Returns whether annotations are drawn within the composer map.

:rtype: bool

.. seealso:: :py:func:`setDrawAnnotations()`
%End

    double mapUnitsToMM() const;
%Docstring
Returns the conversion factor map units -> mm

:rtype: float
%End

    void assignFreeId();
%Docstring
Sets mId to a number not yet used in the composition. mId is kept if it is not in use.
Usually, this function is called before adding the composer map to the composition*
%End

    bool atlasDriven() const;
%Docstring
Returns whether the map extent is set to follow the current atlas feature.

:return: true if map will follow the current atlas feature.
:rtype: bool


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
:rtype: AtlasScalingMode


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

    double atlasMargin( const QgsComposerObject::PropertyValueType valueType = QgsComposerObject::EvaluatedValue );
%Docstring
Returns the margin size (percentage) used when the map is in atlas mode.
:param valueType: controls whether the returned value is the user specified atlas margin,
or the current evaluated atlas margin (which may be affected by data driven atlas margin
settings).

:return: margin size in percentage to leave around the atlas feature's extent
:rtype: float


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

    virtual int numberExportLayers() const;

%Docstring
Get the number of layers that this item requires for exporting as layers

:return: 0 if this item is to be placed on the same layer as the previous item,
:rtype: int

1 if it should be placed on its own layer, and >1 if it requires multiple export layers

.. versionadded:: 2.4
%End

    QPolygonF visibleExtentPolygon() const;
%Docstring
Returns a polygon representing the current visible map extent, considering map extents and rotation.
If the map rotation is 0, the result is the same as currentMapExtent

:return: polygon with the four corner points representing the visible map extent. The points are
:rtype: QPolygonF

clockwise, starting at the top-left point

.. seealso:: :py:func:`currentMapExtent`
%End

    virtual QString displayName() const;


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

    void requestedExtent( QgsRectangle &extent ) const;
%Docstring
Calculates the extent to request and the yShift of the top-left point in case of rotation.

.. versionadded:: 2.6
%End

    virtual QgsExpressionContext createExpressionContext() const;


  signals:
    void extentChanged();

    void mapRotationChanged( double newRotation );
%Docstring
Is emitted on rotation change to notify north arrow pictures
%End

    void preparedForAtlas();
%Docstring
Is emitted when the map has been prepared for atlas rendering, just before actual rendering
%End

    void layerStyleOverridesChanged();
%Docstring
Emitted when layer style overrides are changed... a means to let
associated legend items know they should update

.. versionadded:: 2.10
%End

  public slots:

    void invalidateCache();
%Docstring
Forces a deferred update of the cached map image on next paint.

.. versionadded:: 3.0
%End

    void updateBoundingRect();
%Docstring
Updates the bounding rect of this item. Call this function before doing any changes related to annotation out of the map rectangle
%End

    virtual void refreshDataDefinedProperty( const QgsComposerObject::DataDefinedProperty property = QgsComposerObject::AllProperties, const QgsExpressionContext *context = 0 );


};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/composer/qgscomposermap.h                                   *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
