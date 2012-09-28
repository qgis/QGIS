/** \ingroup analysis
 * The QGis class that calculates raster statistics (count, sum, mean) for
 * a polygon or multipolygon layer and appends the results as attributes
 */

class QgsZonalStatistics
{
%TypeHeaderCode
#include <qgszonalstatistics.h>
%End

  public:

    QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile,
                        const QString& attributePrefix = "", int rasterBand = 1 );
    ~QgsZonalStatistics();

    /**Starts the calculation
      @return 0 in case of success*/
    int calculateStatistics( QProgressDialog* p );
};
