class QgsRasterHistogramWidget : QWidget
{
%TypeHeaderCode
#include <qgsrasterhistogramwidget.h>
%End
  public:
    QgsRasterHistogramWidget( QgsRasterLayer *lyr, QWidget *parent = 0 );
    ~QgsRasterHistogramWidget();

    /** Save the histogram as an image to disk */
    bool histoSaveAsImage( const QString& theFilename, int width = 600, int height = 600, int quality = -1 );

    /** Set the renderer widget (or just its name if there is no widget) */
    void setRendererWidget( const QString& name, QgsRasterRendererWidget* rendererWidget = NULL );

    /** Activate the histogram widget */
    void setActive( bool theActiveFlag );

    /** \brief Compute the histogram on demand. */
    bool computeHistogram( bool forceComputeFlag );

    /** Apply a histoActionTriggered() event. */
    void histoAction( const QString actionName, bool actionFlag = true );

    /** Apply a histoActionTriggered() event. */
    void setSelectedBand( int index );

  public slots:
    /** \brief slot executed when user wishes to refresh raster histogramwidget */
    void refreshHistogram();
    /** This slot lets you save the histogram as an image to disk */
    void on_mSaveAsImageButton_clicked();

  private slots:
    /** Used when the histogram band selector changes, or when tab is loaded. */
    void on_cboHistoBand_currentIndexChanged( int );
    /** Applies the selected min/max values to the renderer widget. */
    void applyHistoMin( );
    void applyHistoMax( );
    /** Button to activate picking of the min/max value on the graph. */
    void on_btnHistoMin_toggled();
    void on_btnHistoMax_toggled();
    /** Called when a selection has been made using the plot picker. */
    void histoPickerSelected( const QPointF & );
    /** Called when a selection has been made using the plot picker (for qwt5 only).
      @note not available in python bindings
      */
    // void histoPickerSelectedQwt5( const QwtDoublePoint & );
    /** Various actions that are stored in btnHistoActions. */
    void histoActionTriggered( QAction* );
    /** Draw the min/max markers on the histogram plot. */
    void updateHistoMarkers();
    /** Button to compute the histogram, appears when no cached histogram is available. */
    void on_btnHistoCompute_clicked();
};
