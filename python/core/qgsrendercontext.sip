
class QgsRenderContext
{

%TypeHeaderCode
#include <qgsrendercontext.h>
%End

  public:
    QgsRenderContext();
    ~QgsRenderContext();

    //getters

    QPainter* painter();
    const QPainter* constPainter() const;

    const QgsCoordinateTransform* coordinateTransform() const;

    const QgsRectangle& extent() const;

    const QgsMapToPixel& mapToPixel() const;

    double scaleFactor() const;

    double rasterScaleFactor() const;

    bool renderingStopped() const;

    bool forceVectorOutput() const;

    /**Returns true if advanced effects such as blend modes such be used
      @note added in 1.9*/
    bool useAdvancedEffects() const;
    /**Used to enable or disable advanced effects such as blend modes
      @note: added in version 1.9*/
    void setUseAdvancedEffects( bool enabled );

    bool drawEditingInformation() const;

    double rendererScale() const;

    //! Added in QGIS v1.4
    QgsLabelingEngineInterface* labelingEngine();

    //! Added in QGIS v2.0
    QColor selectionColor() const;

    //setters

    /**Sets coordinate transformation. QgsRenderContext does not take ownership*/
    void setCoordinateTransform( const QgsCoordinateTransform* t );
    void setMapToPixel( const QgsMapToPixel& mtp );
    void setExtent( const QgsRectangle& extent );
    void setDrawEditingInformation( bool b );
    void setRenderingStopped( bool stopped );
    void setScaleFactor( double factor );
    void setRasterScaleFactor( double factor );
    void setRendererScale( double scale );
    void setPainter( QPainter* p );
    //! Added in QGIS v1.5
    void setForceVectorOutput( bool force );
    //! Added in QGIS v1.4
    void setLabelingEngine(QgsLabelingEngineInterface* iface);
    //! Added in QGIS v2.0
    void setSelectionColor( const QColor& color );

    /**Returns true if the rendering optimization (geometry simplification) can be executed*/
    bool useRenderingOptimization() const;
    void setUseRenderingOptimization( bool enabled );
};
