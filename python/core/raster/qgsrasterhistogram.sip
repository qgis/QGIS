class QgsRasterHistogram
{
%TypeHeaderCode
#include <qgsrasterhistogram.h>
%End
  public:
    QgsRasterHistogram();

    /*! Compares region, size etc. not histogram itself */
    bool operator==( const QgsRasterHistogram &h ) const;

    /** \brief The gdal band number (starts at 1)*/
    int bandNumber;

    /** \brief Number of bins (intervals,buckets) in histogram. */
    int binCount;

    /** \brief The number of non NULL cells used to calculate histogram. */
    int nonNullCount;

    /** \brief Whether histogram includes out of range values (in first and last bin) */
    bool includeOutOfRange;

    /** \brief Store the histogram for a given layer
      * @note not available via python binding
      */
    // HistogramVector histogramVector;

    /** \brief The maximum histogram value. */
    double maximum;

    /** \brief The minimum histogram value. */
    double minimum;

    /** \brief Number of columns used to calc histogram */
    int width;

    /** \brief Number of rows used to calc histogram */
    int height;

    /** \brief Extent used to calc histogram */
    QgsRectangle extent;

    /** \brief Histogram is valid */
    bool valid;
};
