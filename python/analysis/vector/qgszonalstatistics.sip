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

    //! Enumeration of flags that specify statistics to be calculated
    enum Statistic
    {
      Count,  //!< Pixel count
      Sum,  //!< Sum of pixel values
      Mean,  //!< Mean of pixel values
      Median, //!< Median of pixel values
      StDev, //!< Standard deviation of pixel values
      Min,  //!< Min of pixel values
      Max,  //!< Max of pixel values
      Range, //!< Range of pixel values (max - min)
      Minority, //!< Minority of pixel values
      Majority, //!< Majority of pixel values
      Variety, //!< Variety (count of distinct) pixel values
      All
    };

    typedef QFlags<QgsZonalStatistics::Statistic> Statistics;

    QgsZonalStatistics( QgsVectorLayer* polygonLayer, const QString& rasterFile, const QString& attributePrefix = "", int rasterBand = 1,
                        QgsZonalStatistics::Statistics stats = QgsZonalStatistics::Statistics( QgsZonalStatistics::Count | QgsZonalStatistics::Sum | QgsZonalStatistics::Mean) );
    ~QgsZonalStatistics();

    /** Starts the calculation
      @return 0 in case of success*/
    int calculateStatistics( QProgressDialog* p );
};

QFlags<QgsZonalStatistics::Statistic> operator|(QgsZonalStatistics::Statistic f1, QFlags<QgsZonalStatistics::Statistic> f2);

