class QgsRasterLayerSaveAsDialog : QDialog
{
%TypeHeaderCode
#include <qgsextentgroupbox.h>
#include <qgsrasterlayersaveasdialog.h>
%End
  public:
    enum Mode
    {
      RawDataMode,
      RenderedImageMode
    };
    enum CrsState
    {
      OriginalCrs,
      CurrentCrs,
      UserCrs
    };
    enum ResolutionState
    {
      OriginalResolution,
      UserResolution
    };

    QgsRasterLayerSaveAsDialog( QgsRasterLayer* rasterLayer,
                                QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent,
                                const QgsCoordinateReferenceSystem& layerCrs, const QgsCoordinateReferenceSystem& currentCrs,
                                QWidget* parent /TransferThis/ = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterLayerSaveAsDialog();

    Mode mode() const;
    int nColumns() const;
    int nRows() const;
    double xResolution() const;
    double yResolution() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;
    QString outputFileName() const;
    QString outputFormat() const;
    QgsCoordinateReferenceSystem outputCrs();
    QStringList createOptions() const;
    QgsRectangle outputRectangle() const;
    QgsRasterRangeList noData() const;

    QList< int > pyramidsList() const;
    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
    QString pyramidsResamplingMethod() const;
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
    QStringList pyramidsConfigOptions() const;

    void hideFormat();
    void hideOutput();

  public slots:
    virtual void accept();
};
