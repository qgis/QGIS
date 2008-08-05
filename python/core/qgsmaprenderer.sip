
/**
 * \class QgsMapRenderer
 * \brief Class for rendering map layer set
 *
 */

class QgsMapRenderer : QObject
{
%TypeHeaderCode
#include <qgsmaprenderer.h>
%End
      
  public:
    
    //! constructor
    QgsMapRenderer();
    
    //! destructor
    ~QgsMapRenderer();

    //! starts rendering
    void render(QPainter* painter);
    
    //! sets extent and checks whether suitable (returns false if not)
    bool setExtent(const QgsRect& extent);
    
    //! returns current extent
    QgsRect extent();
    
    const QgsMapToPixel* coordXForm();
    
    double scale() const;
    double mupp() const;

    int width() const;
    int height() const;


    //! Recalculate the map scale
    void updateScale();

    QGis::units mapUnits() const;
    void setMapUnits(QGis::units u);
    
    //! sets whether map image will be for overview
    void setOverview(bool isOverview = true);

    void setOutputSize(QSize size, int dpi);
    
    //!accessor for output dpi
    int outputDpi();
    //!accessor for output size
    QSize outputSize();
    
    //! transform extent in layer's SRS to extent in output SRS
    QgsRect layerExtentToOutputExtent(QgsMapLayer* theLayer, QgsRect extent);
    
    //! transform coordinates from layer's SRS to output SRS
    QgsPoint layerCoordsToOutputCoords(QgsMapLayer* theLayer, QgsPoint point);

    //! transform coordinates from output SRS to layer's SRS
    QgsPoint outputCoordsToLayerCoords(QgsMapLayer* theLayer, QgsPoint point);

    //! transform rect's coordinates from output SRS to layer's SRS
    QgsRect outputCoordsToLayerCoords(QgsMapLayer* theLayer, QgsRect rect);
    
    //! sets whether to use projections for this layer set
    void setProjectionsEnabled(bool enabled);
    
    //! returns true if projections are enabled for this layer set
    bool projectionsEnabled();
    
    //! sets destination spatial reference system
    void setDestinationSrs(const QgsSpatialRefSys& srs);
    
    //! returns SRS ID of destination spatial reference system
    const QgsSpatialRefSys& destinationSrs();

    //! returns current extent of layer set
    QgsRect fullExtent();
    
    //! returns current layer set
    QStringList& layerSet();
    
    //! change current layer set
    void setLayerSet(const QStringList& layers);

    //! updates extent of the layer set
    void updateFullExtent();

    //! read settings
    bool readXML(QDomNode & theNode);

    //! write settings
    bool writeXML(QDomNode & theNode, QDomDocument & theDoc);

  signals:
    
    void setProgress(int current, int total);
    
    void projectionsEnabled(bool flag);
    
    void destinationSrsChanged();
    
    void updateMap();
    
    void mapUnitsChanged();

    //! emitted when layer's draw() returned FALSE
    void drawError(QgsMapLayer*);

  public slots:
    
    //! called by signal from layer current being drawn
    void onDrawingProgress(int current, int total);

  protected:
    
    //! adjust extent to fit the pixmap size
    void adjustExtentToSize();
    
    /** Convenience function to project an extent into the layer source
     * SRS, but also split it into two extents if it crosses
     * the +/- 180 degree line. Modifies the given extent to be in the
     * source SRS coordinates, and if it was split, returns true, and
     * also sets the contents of the r2 parameter
     */
    bool splitLayersExtent(QgsMapLayer* layer, QgsRect& extent, QgsRect& r2);

};

