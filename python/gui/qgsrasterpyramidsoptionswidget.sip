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

    QStringList createOptions() const;
    QgsRasterFormatSaveOptionsWidget* createOptionsWidget() /Factory/;
    const QList<int> overviewList() const;
    QgsRasterDataProvider::RasterPyramidsFormat pyramidsFormat() const;
    QString resamplingMethod() const;

  public slots:

    void apply();
    void checkAllLevels( bool checked );

  signals:
    void overviewListChanged();
};
