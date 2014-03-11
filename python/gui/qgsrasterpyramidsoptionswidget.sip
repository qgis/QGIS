/** \ingroup gui
 * A widget to select format-specific raster saving options
 */
class QgsRasterPyramidsOptionsWidget: QWidget
{
%TypeHeaderCode
#include <qgsrasterpyramidsoptionswidget.h>
%End

  public:

    QgsRasterPyramidsOptionsWidget( QWidget* parent = 0, QString provider = "gdal" );
    ~QgsRasterPyramidsOptionsWidget();

    QStringList configOptions() const;
    QgsRasterFormatSaveOptionsWidget* createOptionsWidget() /Factory/;
    const QList<int> overviewList() const;
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
    QString resamplingMethod() const;
    void setRasterLayer( QgsRasterLayer* rasterLayer );
    void setRasterFileName( const QString& file );

  public slots:

    void apply();
    void checkAllLevels( bool checked );

  private slots:

    void on_cbxPyramidsLevelsCustom_toggled( bool toggled );
    void on_cbxPyramidsFormat_currentIndexChanged( int index );
    void setOverviewList();
    void updateUi();

  signals:
    void overviewListChanged();
    void someValueChanged(); /* emitted when any other setting changes */
};
