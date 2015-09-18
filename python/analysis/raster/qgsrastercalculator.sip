struct QgsRasterCalculatorEntry
{
%TypeHeaderCode
#include <qgsrastercalculator.h>
%End

  QString ref; //name
  QgsRasterLayer* raster; //pointer to rasterlayer
  int bandNumber; //raster band number
};

/** Raster calculator class*/
class QgsRasterCalculator
{
%TypeHeaderCode
#include <qgsrastercalculator.h>
%End

  public:

    /** QgsRasterCalculator constructor.
     * @param formulaString formula for raster calculation
     * @param outputFile output file path
     * @param outputFormat output file format
     * @param outputExtent output extent. CRS for output is taken from first entry in rasterEntries.
     * @param nOutputColumns number of columns in output raster
     * @param nOutputRows number of rows in output raster
     * @param rasterEntries list of referenced raster layers
     */
    QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
                         const QgsRectangle& outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries );

    /** QgsRasterCalculator constructor.
     * @param formulaString formula for raster calculation
     * @param outputFile output file path
     * @param outputFormat output file format
     * @param outputExtent output extent, CRS is specified by outputCrs parameter
     * @param outputCrs destination CRS for output raster
     * @param nOutputColumns number of columns in output raster
     * @param nOutputRows number of rows in output raster
     * @param rasterEntries list of referenced raster layers
     * @note added in QGIS 2.10
     */
    QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
                         const QgsRectangle& outputExtent, const QgsCoordinateReferenceSystem& outputCrs, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries );

    ~QgsRasterCalculator();

    /** Starts the calculation and writes new raster
      @param p progress bar (or 0 if called from non-gui code)
      @return 0 in case of success*/
    int processCalculation( QProgressDialog* p = 0 );
};
