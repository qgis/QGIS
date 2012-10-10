struct QgsRasterCalculatorEntry
{
%TypeHeaderCode
#include <qgsrastercalculator.h>
%End

  QString ref; //name
  QgsRasterLayer* raster; //pointer to rasterlayer
  int bandNumber; //raster band number
};

/**Raster calculator class*/
class QgsRasterCalculator
{
%TypeHeaderCode
#include <qgsrastercalculator.h>
%End

  public:
    QgsRasterCalculator( const QString& formulaString, const QString& outputFile, const QString& outputFormat,
                         const QgsRectangle& outputExtent, int nOutputColumns, int nOutputRows, const QVector<QgsRasterCalculatorEntry>& rasterEntries );
    ~QgsRasterCalculator();

    /**Starts the calculation and writes new raster
      @param p progress bar (or 0 if called from non-gui code)
      @return 0 in case of success*/
    int processCalculation( QProgressDialog* p = 0 );
};
