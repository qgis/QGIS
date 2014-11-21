

class QgsLabelPosition
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End
  public:
    QgsLabelPosition( int id, double r, const QVector< QgsPoint >& corners, const QgsRectangle& rect, double w, double h, const QString& layer, const QString& labeltext, const QFont& labelfont, bool upside_down, bool diagram = false, bool pinned = false );
    QgsLabelPosition();
    int featureId;
    double rotation;
    QVector< QgsPoint > cornerPoints;
    QgsRectangle labelRect;
    double width;
    double height;
    QString layerID;
    QString labelText;
    QFont labelFont;
    bool upsideDown;
    bool isDiagram;
    bool isPinned;
};

/** Labeling engine interface. */
class QgsLabelingEngineInterface
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End

  public:

    virtual ~QgsLabelingEngineInterface();

    //! called when we're going to start with rendering
    //! @deprecated since 2.4 - use override with QgsMapSettings
    virtual void init( QgsMapRenderer* mp ) = 0 /Deprecated/;
    //! called when we're going to start with rendering
    virtual void init( const QgsMapSettings& mapSettings ) = 0;
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer ) = 0;
    //! clears all PAL layer settings for registered layers
    virtual void clearActiveLayers() = 0;
    //! clears data defined objects from PAL layer settings for a registered layer
    virtual void clearActiveLayer( const QString& layerID ) = 0;
    //! called when starting rendering of a layer
    virtual int prepareLayer( QgsVectorLayer* layer, QStringList& attrNames, QgsRenderContext& ctx ) = 0;
    //! returns PAL layer settings for a registered layer
    virtual QgsPalLayerSettings& layer( const QString& layerName ) = 0;
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, const QgsDiagramLayerSettings* s );
    //! called for every feature
    virtual void registerFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext(), QString dxfLayer = QString::null ) = 0;
    //! called for every diagram feature
    virtual void registerDiagramFeature( const QString& layerID, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() );
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context ) = 0;
    //! called when we're done with rendering
    virtual void exit() = 0;
    //! return infos about labels at a given (map) position
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) = 0 /Deprecated/;
    //! return infos about labels within a given (map) rectangle
    //! @deprecated since 2.4 - use takeResults() and methods of QgsLabelingResults
    virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) = 0 /Deprecated/;

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() = 0 /Factory/;
};

struct QgsLayerCoordinateTransform
{
  QString srcAuthId;
  QString destAuthId;
  int srcDatumTransform; //-1 if unknown or not specified
  int destDatumTransform;
};

// ### QGIS 3: remove QgsMapRenderer in favor of QgsMapRendererJob

/** \ingroup core
 * A non GUI class for rendering a map layer set onto a QPainter.
 */

class QgsMapRenderer : QObject
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End

  public:

    /**Output units for pen width and point marker width/height*/
    enum OutputUnits
    {
      Millimeters,
      Pixels
      //MAP_UNITS probably supported in future versions
    };

    /** Blending modes enum defining the available composition modes that can
     * be used when rendering a layer
     */
    enum BlendMode
    {
      BlendNormal,
      BlendLighten,
      BlendScreen,
      BlendDodge,
      BlendAddition,
      BlendDarken,
      BlendMultiply,
      BlendBurn,
      BlendOverlay,
      BlendSoftLight,
      BlendHardLight,
      BlendDifference,
      BlendSubtract,
      BlendSource,
      BlendDestinationOver,
      BlendClear,
      BlendDestination,
      BlendSourceIn,
      BlendDestinationIn,
      BlendSourceOut,
      BlendDestinationOut,
      BlendSourceAtop,
      BlendDestinationAtop,
      BlendXor,
    };

    //! constructor
    QgsMapRenderer();

    //! destructor
    ~QgsMapRenderer();

    //! starts rendering
    //! @param painter painter to render to
    //! @param forceWidthScale Force a specific scale factor for line widths and marker sizes. Automatically calculated from output device DPI if 0
    void render( QPainter* painter, double* forceWidthScale = 0 );

    //! sets extent and checks whether suitable (returns false if not)
    bool setExtent( const QgsRectangle& extent );

    //! returns current extent
    QgsRectangle extent() const;

    const QgsMapToPixel* coordinateTransform();

    //! Scale denominator
    double scale() const;
    /**Sets scale for scale based visibility. Normally, the scale is calculated automatically. This
     function is only used to force a preview scale (e.g. for print composer)*/
    void setScale( double scale );
    double mapUnitsPerPixel() const;

    int width() const;
    int height() const;

    //! Recalculate the map scale
    void updateScale();

    QGis::UnitType mapUnits() const;
    void setMapUnits( QGis::UnitType u );

    //! sets whether map image will be for overview
    void enableOverviewMode( bool isOverview = true );

    void setOutputSize( QSize size, int dpi );
    void setOutputSize( QSizeF size, double dpi );

    //!accessor for output dpi
    double outputDpi();
    //!accessor for output size
    QSize outputSize();
    QSizeF outputSizeF();

    /**
     * @brief transform bounding box from layer's CRS to output CRS
     * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    /**
     * @brief transform bounding box from output CRS to layer's CRS
     * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent );

    /**
     * @brief transform point coordinates from layer's CRS to output CRS
     * @return the transformed point
     */
    QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    /**
     * @brief transform rectangle from layer's CRS to output CRS
     * @see layerExtentToOutputExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

    /**
     * @brief transform point coordinates from output CRS to layer's CRS
     * @return the transformed point
     */
    QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point );

    /**
     * @brief transform rectangle from output CRS to layer's CRS
     * @see outputExtentToLayerExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect );

    //! sets whether to use projections for this layer set
    void setProjectionsEnabled( bool enabled );

    //! returns true if projections are enabled for this layer set
    bool hasCrsTransformEnabled() const;

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs, bool refreshCoordinateTransformInfo = true, bool transformExtent = true );

    //! returns CRS of destination coordinate reference system
    const QgsCoordinateReferenceSystem& destinationCrs() const;

    void setOutputUnits( OutputUnits u );

    OutputUnits outputUnits() const;

    //! returns current extent of layer set
    QgsRectangle fullExtent();

    //! returns current layer set
    QStringList& layerSet();

    //! change current layer set
    void setLayerSet( const QStringList& layers );

    //! updates extent of the layer set
    void updateFullExtent();

    //! read settings
    bool readXML( QDomNode & theNode );

    //! write settings
    bool writeXML( QDomNode & theNode, QDomDocument & theDoc );

    //! Accessor for render context
    QgsRenderContext* rendererContext();

    //! Labeling engine (NULL if there's no custom engine)
    QgsLabelingEngineInterface* labelingEngine();

    //! Set labeling engine. Previous engine (if any) is deleted.
    //! Takes ownership of the engine.
    void setLabelingEngine( QgsLabelingEngineInterface* iface /Transfer/ );

    //! Returns a QPainter::CompositionMode corresponding to a BlendMode
    static QPainter::CompositionMode getCompositionMode( const QgsMapRenderer::BlendMode &blendMode );
    //! Returns a BlendMode corresponding to a QPainter::CompositionMode
    static QgsMapRenderer::BlendMode getBlendModeEnum( const QPainter::CompositionMode &blendMode );

    void addLayerCoordinateTransform( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );
    void clearLayerCoordinateTransforms();

    const QgsCoordinateTransform* transformation( const QgsMapLayer *layer ) const;

    //! bridge to QgsMapSettings
    //! @note added in 2.4
    const QgsMapSettings& mapSettings();

    /** Convenience function to project an extent into the layer source
     * CRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source CRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    bool splitLayersExtent( QgsMapLayer* layer, QgsRectangle& extent /In,Out/, QgsRectangle& r2 /Out/ );

  signals:

    //! @deprecated in 2.4 - not emitted anymore
    void drawingProgress( int current, int total );

    /** This signal is emitted when CRS transformation is enabled/disabled.
     *  @param flag true if transformation is enabled.
     *  @deprecated Use hasCrsTransformEnabledChanged( bool flag )
     *              to avoid conflict with method of the same name). */
    void hasCrsTransformEnabled( bool flag ) /Deprecated/;

    /** This signal is emitted when CRS transformation is enabled/disabled.
     *  @param flag true if transformation is enabled.
     *  @note Added in 2.4 */
    void hasCrsTransformEnabledChanged( bool flag );

    void destinationSrsChanged();

    //! @deprecated in 2.4 - not emitted anymore
    void updateMap();

    void mapUnitsChanged();

    //! emitted when layer's draw() returned false
    void drawError( QgsMapLayer* );

    //! emitted when the current extent gets changed
    //! @note added in 2.4
    void extentsChanged();

    //! Notifies higher level components to show the datum transform dialog and add a QgsLayerCoordinateTransformInfo for that layer
    void datumTransformInfoRequested( const QgsMapLayer* ml, const QString& srcAuthId, const QString& destAuthId ) const;


  public slots:

    //! @deprecated in 2.4 - does nothing
    void onDrawingProgress( int current, int total ) /Deprecated/;

  protected:

    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();

};
