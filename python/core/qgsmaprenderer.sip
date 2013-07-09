

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

/** Labeling engine interface.
 * \note Added in QGIS v1.4
 */
class QgsLabelingEngineInterface
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End

  public:

    virtual ~QgsLabelingEngineInterface();

    //! called when we're going to start with rendering
    virtual void init( QgsMapRenderer* mp ) = 0;
    //! called to find out whether the layer is used for labeling
    virtual bool willUseLayer( QgsVectorLayer* layer ) = 0;
    //! clears all PAL layer settings for registered layers
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayers() = 0;
    //! clears data defined objects from PAL layer settings for a registered layer
    //! @note: this method was added in version 1.9
    virtual void clearActiveLayer( QgsVectorLayer* layer ) = 0;
    //! called when starting rendering of a layer
    //! @note: this method was added in version 1.6
    virtual int prepareLayer( QgsVectorLayer* layer, QSet<int>& attrIndices, QgsRenderContext& ctx ) = 0;
    //! returns PAL layer settings for a registered layer
    //! @note: this method was added in version 1.9
    virtual QgsPalLayerSettings& layer( const QString& layerName ) = 0;
    //! adds a diagram layer to the labeling engine
    virtual int addDiagramLayer( QgsVectorLayer* layer, QgsDiagramLayerSettings* s );
    //! called for every feature
    virtual void registerFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() ) = 0;
    //! called for every diagram feature
    virtual void registerDiagramFeature( QgsVectorLayer* layer, QgsFeature& feat, const QgsRenderContext& context = QgsRenderContext() );
    //! called when the map is drawn and labels should be placed
    virtual void drawLabeling( QgsRenderContext& context ) = 0;
    //! called when we're done with rendering
    virtual void exit() = 0;
    //! return infos about labels at a given (map) position
    //! @note: this method was added in version 1.7
    virtual QList<QgsLabelPosition> labelsAtPosition( const QgsPoint& p ) = 0;
    //! return infos about labels within a given (map) rectangle
    //! @note: this method was added in version 1.9
    virtual QList<QgsLabelPosition> labelsWithinRect( const QgsRectangle& r ) = 0;

    //! called when passing engine among map renderers
    virtual QgsLabelingEngineInterface* clone() = 0 /Factory/;
};



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
      BlendSubtract
    };

    //! constructor
    QgsMapRenderer();

    //! destructor
    ~QgsMapRenderer();

    //! starts rendering
    //! @ param forceWidthScale Force a specific scale factor for line widths and marker sizes. Automatically calculated from output device DPI if 0
    void render( QPainter* painter );

    //! sets extent and checks whether suitable (returns false if not)
    bool setExtent( const QgsRectangle& extent );

    //! returns current extent
    QgsRectangle extent() const;

    const QgsMapToPixel* coordinateTransform();

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
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );

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
    //! \note Added in QGIS v1.4
    QgsLabelingEngineInterface* labelingEngine();

    //! Set labeling engine. Previous engine (if any) is deleted.
    //! Takes ownership of the engine.
    //! Added in QGIS v1.4
    void setLabelingEngine( QgsLabelingEngineInterface* iface /Transfer/ );

  signals:

    void drawingProgress( int current, int total );

    void hasCrsTransformEnabled( bool flag );

    void destinationSrsChanged();

    void updateMap();

    void mapUnitsChanged();

    //! emitted when layer's draw() returned false
    void drawError( QgsMapLayer* );

  public slots:

    //! called by signal from layer current being drawn
    void onDrawingProgress( int current, int total );

  protected:

    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();

    /** Convenience function to project an extent into the layer source
     * CRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source CRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    bool splitLayersExtent( QgsMapLayer* layer, QgsRectangle& extent, QgsRectangle& r2 );
};
