
class QgsMapSettings
{
%TypeHeaderCode
#include <qgsmapsettings.h>
%End

  public:
    QgsMapSettings();

    //! Return geographical coordinates of the rectangle that should be rendered.
    //! The actual visible extent used for rendering could be slightly different
    //! since the given extent may be expanded in order to fit the aspect ratio
    //! of output size. Use visibleExtent() to get the resulting extent.
    QgsRectangle extent() const;
    //! Set coordinates of the rectangle which should be rendered.
    //! The actual visible extent used for rendering could be slightly different
    //! since the given extent may be expanded in order to fit the aspect ratio
    //! of output size. Use visibleExtent() to get the resulting extent.
    void setExtent( const QgsRectangle& rect );

    //! Return the size of the resulting map image
    QSize outputSize() const;
    //! Set the size of the resulting map image
    void setOutputSize( const QSize& size );

    //! Return DPI used for conversion between real world units (e.g. mm) and pixels
    //! Default value is 96
    int outputDpi() const;
    //! Set DPI used for conversion between real world units (e.g. mm) and pixels
    void setOutputDpi( int dpi );

    //! Get list of layer IDs for map rendering
    //! The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
    QStringList layers() const;
    //! Set list of layer IDs for map rendering. The layers must be registered in QgsMapLayerRegistry.
    //! The layers are stored in the reverse order of how they are rendered (layer with index 0 will be on top)
    void setLayers( const QStringList& layers );

    //! sets whether to use projections for this layer set
    void setCrsTransformEnabled( bool enabled );
    //! returns true if projections are enabled for this layer set
    bool hasCrsTransformEnabled() const;

    //! sets destination coordinate reference system
    void setDestinationCrs( const QgsCoordinateReferenceSystem& crs );
    //! returns CRS of destination coordinate reference system
    const QgsCoordinateReferenceSystem& destinationCrs() const;

    //! Get units of map's geographical coordinates - used for scale calculation
    QGis::UnitType mapUnits() const;
    //! Set units of map's geographical coordinates - used for scale calculation
    void setMapUnits( QGis::UnitType u );

    //! Set the background color of the map
    void setBackgroundColor( const QColor& color );
    //! Get the background color of the map
    QColor backgroundColor() const;

    //! Set color that is used for drawing of selected vector features
    void setSelectionColor( const QColor& color );
    //! Get color that is used for drawing of selected vector features
    QColor selectionColor() const;

    //! Enumeration of flags that adjust the way how map is rendered
    enum Flag
    {
      Antialiasing,               //!< Enable anti-aliasin for map rendering
      DrawEditingInfo,            //!< Enable drawing of vertex markers for layers in editing mode
      ForceVectorOutput,          //!< Vector graphics should not be cached and drawn as raster images
      UseAdvancedEffects,         //!< Enable layer transparency and blending effects
      DrawLabeling,               //!< Enable drawing of labels on top of the map
      UseRenderingOptimization,        //!< Enable vector simplification and other rendering optimizations
      DrawSelection,              //!< Whether vector selections should be shown in the rendered map
      // TODO: ignore scale-based visibility (overview)
    };
    typedef QFlags<QgsMapSettings::Flag> Flags;

    //! Set combination of flags that will be used for rendering
    void setFlags( Flags flags );
    //! Enable or disable a particular flag (other flags are not affected)
    void setFlag( Flag flag, bool on = true );
    //! Return combination of flags used for rendering
    Flags flags() const;
    //! Check whether a particular flag is enabled
    bool testFlag( Flag flag ) const;

    //! sets format of internal QImage
    void setOutputImageFormat( QImage::Format format );
    //! format of internal QImage, default QImage::Format_ARGB32_Premultiplied
    QImage::Format outputImageFormat() const;

    //! Check whether the map settings are valid and can be used for rendering
    bool hasValidSettings() const;
    //! Return the actual extent derived from requested extent that takes takes output image size into account
    QgsRectangle visibleExtent() const;
    //! Return the distance in geographical coordinates that equals to one pixel in the map
    double mapUnitsPerPixel() const;
    //! Return the calculated scale of the map
    double scale() const;


    // -- utility functions --

    //const QgsDatumTransformStore& datumTransformStore() const;
    QgsDatumTransformStore& datumTransformStore();

    const QgsMapToPixel& mapToPixel() const;

    /**
     * @brief transform bounding box from layer's CRS to output CRS
     * @see layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle layerExtentToOutputExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform bounding box from output CRS to layer's CRS
     * @see mapToLayerCoordinates( QgsMapLayer* theLayer,QgsRectangle rect ) if you want to transform a rectangle
     * @return a bounding box (aligned rectangle) containing the transformed extent
     */
    QgsRectangle outputExtentToLayerExtent( QgsMapLayer* theLayer, QgsRectangle extent ) const;

    /**
     * @brief transform point coordinates from layer's CRS to output CRS
     * @return the transformed point
     */
    QgsPoint layerToMapCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from layer's CRS to output CRS
     * @see layerExtentToOutputExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle layerToMapCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;

    /**
     * @brief transform point coordinates from output CRS to layer's CRS
     * @return the transformed point
     */
    QgsPoint mapToLayerCoordinates( QgsMapLayer* theLayer, QgsPoint point ) const;

    /**
     * @brief transform rectangle from output CRS to layer's CRS
     * @see outputExtentToLayerExtent() if you want to transform a bounding box
     * @return the transformed rectangle
     */
    QgsRectangle mapToLayerCoordinates( QgsMapLayer* theLayer, QgsRectangle rect ) const;

    /**
     * @brief Return coordinate transform from layer's CRS to destination CRS
     * @param layer
     * @return transform - may be null if the transform is not needed
     */
    const QgsCoordinateTransform* layerTransfrom( QgsMapLayer *layer ) const;

    //! returns current extent of layer set
    QgsRectangle fullExtent() const;

    /* serialization */

    void readXML( QDomNode& theNode );

    void writeXML( QDomNode& theNode, QDomDocument& theDoc );

};

QFlags<QgsMapSettings::Flag> operator|(QgsMapSettings::Flag f1, QFlags<QgsMapSettings::Flag> f2);
