/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterlayersaveasdialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/



class QgsRasterLayerSaveAsDialog: QDialog
{

%TypeHeaderCode
#include "qgsrasterlayersaveasdialog.h"
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

    QgsRasterLayerSaveAsDialog( QgsRasterLayer *rasterLayer,
                                QgsRasterDataProvider *sourceProvider,
                                const QgsRectangle &currentExtent,
                                const QgsCoordinateReferenceSystem &layerCrs,
                                const QgsCoordinateReferenceSystem &currentCrs,
                                QWidget *parent /TransferThis/ = 0,
                                Qt::WindowFlags f = 0 );

    Mode mode() const;
%Docstring
 :rtype: Mode
%End
    int nColumns() const;
%Docstring
 :rtype: int
%End
    int nRows() const;
%Docstring
 :rtype: int
%End
    double xResolution() const;
%Docstring
 :rtype: float
%End
    double yResolution() const;
%Docstring
 :rtype: float
%End
    int maximumTileSizeX() const;
%Docstring
 :rtype: int
%End
    int maximumTileSizeY() const;
%Docstring
 :rtype: int
%End
    bool tileMode() const;
%Docstring
 :rtype: bool
%End
    bool addToCanvas() const;
%Docstring
 :rtype: bool
%End
    QString outputFileName() const;
%Docstring
 :rtype: str
%End
    QString outputFormat() const;
%Docstring
 :rtype: str
%End
    QgsCoordinateReferenceSystem outputCrs();
%Docstring
 :rtype: QgsCoordinateReferenceSystem
%End
    QStringList createOptions() const;
%Docstring
 :rtype: list of str
%End
    QgsRectangle outputRectangle() const;
%Docstring
 :rtype: QgsRectangle
%End
    QgsRasterRangeList noData() const;
%Docstring
 :rtype: QgsRasterRangeList
%End

    QList< int > pyramidsList() const;
%Docstring
 :rtype: list of int
%End
    QgsRaster::RasterBuildPyramids buildPyramidsFlag() const;
%Docstring
 :rtype: QgsRaster.RasterBuildPyramids
%End
    QString pyramidsResamplingMethod() const;
%Docstring
 :rtype: str
%End
    QgsRaster::RasterPyramidsFormat pyramidsFormat() const;
%Docstring
 :rtype: QgsRaster.RasterPyramidsFormat
%End
    QStringList pyramidsConfigOptions() const;
%Docstring
 :rtype: list of str
%End

    void hideFormat();
    void hideOutput();

  public slots:
    virtual void accept();

};



/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsrasterlayersaveasdialog.h                                 *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
