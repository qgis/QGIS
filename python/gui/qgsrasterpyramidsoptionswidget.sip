/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterpyramidsoptionswidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsRasterPyramidsOptionsWidget: QWidget
{
%Docstring
 A widget to select format-specific raster saving options
%End

%TypeHeaderCode
#include "qgsrasterpyramidsoptionswidget.h"
%End
  public:

    QgsRasterPyramidsOptionsWidget( QWidget *parent /TransferThis/ = 0, const QString &provider = "gdal" );

    QStringList configOptions() const;
%Docstring
 :rtype: list of str
%End
    QgsRasterFormatSaveOptionsWidget *createOptionsWidget() /Factory/;
%Docstring
 :rtype: QgsRasterFormatSaveOptionsWidget
%End
    const QList<int> overviewList() const;
%Docstring
 :rtype: list of int
%End
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
%Docstring
 :rtype: QgsRaster.RasterPyramidsFormat
%End
    QString resamplingMethod() const;
%Docstring
 :rtype: str
%End
    void setRasterLayer( QgsRasterLayer *rasterLayer );
    void setRasterFileName( const QString &file );

  public slots:

    void apply();
    void checkAllLevels( bool checked );

  private slots:
    void cbxPyramidsLevelsCustom_toggled( bool toggled ) ;
    void cbxPyramidsFormat_currentIndexChanged( int index ) ;
    void setOverviewList() ;
    void updateUi() ;
  signals:
    void overviewListChanged();
    void someValueChanged(); /* emitted when any other setting changes */

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterpyramidsoptionswidget.h                             *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
