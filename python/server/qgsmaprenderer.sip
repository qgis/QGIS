
struct QgsLayerCoordinateTransform
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End

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

    /** Output units for pen width and point marker width/height*/
    enum OutputUnits
    {
      Millimeters,
      Pixels
      //MAP_UNITS probably supported in future versions
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

    //! sets rotation
    //! value in clockwise degrees
    //! @note added in 2.8
    void setRotation( double degrees );

    //! returns current rotation in clockwise degrees
    //! @note added in 2.8
    double rotation() const;

    const QgsMapToPixel* coordinateTransform();

    //! Scale denominator
    double scale() const;
    /** Sets scale for scale based visibility. Normally, the scale is calculated automatically. This
     function is only used to force a preview scale (e.g. for print composer)*/
    void setScale( double scale );
    double mapUnitsPerPixel() const;

    int width() const;
    int height() const;

    //! Recalculate the map scale
    void updateScale();

    /** Returns the distance units which are used for map units.
     * @see setMapUnits()
     */
    QgsUnitTypes::DistanceUnit mapUnits() const;

    /** Sets the distance units which will be used for map units.
     * @see mapUnits()
     */
    void setMapUnits( QgsUnitTypes::DistanceUnit u );

    //! sets whether map image will be for overview
    void enableOverviewMode( bool isOverview = true );

    /** Sets the desired size of the rendered map image.
     * @param size size in pixels
     * @param dpi resolution to render map using
     * @see outputSize()
     */
    void setOutputSize( QSize size, double dpi );

    /** Sets the desired size of the rendered map image.
     * @param size size in pixels
     * @param dpi resolution to render map using
     * @see outputSizeF()
     */
    void setOutputSize( QSizeF size, double dpi );

    //!accessor for output dpi
    double outputDpi();

    /** Returns the size which the map will be rendered at.
     * @see setOutputSize()
     * @see outputSizeF()
     */
    QSize outputSize() const;

    /** Returns the size which the map will be rendered at.
     * @see setOutputSize()
     * @see outputSize()
     */
    QSizeF outputSizeF() const;

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
    bool readXml( QDomNode & theNode );

    //! write settings
    bool writeXml( QDomNode & theNode, QDomDocument & theDoc );

    //! Accessor for render context
    QgsRenderContext* rendererContext();

    //! Labeling engine (nullptr if there's no custom engine)
    QgsLabelingEngineInterface* labelingEngine();

    //! Set labeling engine. Previous engine (if any) is deleted.
    //! Takes ownership of the engine.
    void setLabelingEngine( QgsLabelingEngineInterface* iface /Transfer/ );

    void addLayerCoordinateTransform( const QString& layerId, const QString& srcAuthId, const QString& destAuthId, int srcDatumTransform = -1, int destDatumTransform = -1 );
    void clearLayerCoordinateTransforms();

    /** Returns the coordinate transform associated with a renderered layer,
     * or an invalid transform is no transform is required for the layer.
     */
    QgsCoordinateTransform transformation( const QgsMapLayer *layer ) const;

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

    /** Set a feature filter provider to filter the features shown in the map.
     * @param ffp the feature filter provider
     * @note added in QGIS 2.14
     */
    void setFeatureFilterProvider( const QgsFeatureFilterProvider* ffp );

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

    //! Emitted when the renderer's destination CRS is changed
    void destinationCrsChanged();

    //! @deprecated in 2.4 - not emitted anymore
    void updateMap();

    void mapUnitsChanged();

    //! emitted when layer's draw() returned false
    void drawError( QgsMapLayer* );

    //! emitted when the current extent gets changed
    //! @note added in 2.4
    void extentsChanged();

    //! emitted when the current rotation gets changed
    //! @note added in 2.8
    void rotationChanged( double );

    //! Notifies higher level components to show the datum transform dialog and add a QgsLayerCoordinateTransformInfo for that layer
    //! @note not available in Python bindings
    //void datumTransformInfoRequested( const QgsMapLayer* ml, const QString& srcAuthId, const QString& destAuthId ) const;

  public slots:

    //! @deprecated in 2.4 - does nothing
    void onDrawingProgress( int current, int total ) /Deprecated/;

  protected:

    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();

};
