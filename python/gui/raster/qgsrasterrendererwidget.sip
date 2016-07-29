class QgsRasterRendererWidget: QWidget
{
%TypeHeaderCode
#include <qgsrasterrendererwidget.h>
%End
  public:
    QgsRasterRendererWidget( QgsRasterLayer* layer, const QgsRectangle &extent );
    virtual ~QgsRasterRendererWidget();

    enum LoadMinMaxAlgo
    {
      Estimate,
      Actual,
      CurrentExtent,
      CumulativeCut   // 2 - 98% cumulative cut
    };

    virtual QgsRasterRenderer* renderer() = 0 /Factory/;

    void setRasterLayer( QgsRasterLayer* layer );
    const QgsRasterLayer* rasterLayer() const;

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map extent and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.16
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas()
     * @see canvasExtent()
     * @note added in QGIS 2.16
     */
    QgsMapCanvas* mapCanvas();

    virtual QString min( int index = 0 );
    virtual QString max( int index = 0 );
    virtual void setMin( const QString& value, int index = 0 );
    virtual void setMax( const QString& value, int index = 0 );
    virtual QString stdDev();
    virtual void setStdDev( const QString& value );
    virtual int selectedBand( int index = 0 );

  signals:
    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();


  protected:
    /** Returns a band name for display. First choice is color name, otherwise band number*/
    QString displayBandName( int band ) const;
};
