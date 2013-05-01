
/** \brief The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class QgsRasterBandStats
{
%TypeHeaderCode
#include <qgsrasterbandstats.h>
%End

  public:
    enum Stats
    {
      None         = 0,
      Min          = 1,
      Max          = 2,
      Range        = 4,
      Sum          = 8,
      Mean         = 16,
      StdDev       = 32,
      SumOfSquares = 64,
      All          = 127
    };

    QgsRasterBandStats();

    /*! Compares region, size etc. not collected statistics */
    bool contains( const QgsRasterBandStats &s ) const;

    /** \brief The gdal band number (starts at 1)*/
    int bandNumber;

    /** Color table */
    // QList<QgsColorRampShader::ColorRampItem> colorTable;

    /** \brief The number of cells in the band. Equivalent to height x width.
     * TODO: check if NO_DATA are excluded!*/
    int elementCount;

    /** \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function. */
    double maximumValue;

    /** \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function. */
    double minimumValue;

    /** \brief The mean cell value for the band. NO_DATA values are excluded. */
    double mean;

    /** \brief The range is the distance between min & max. */
    double range;

    /** \brief The standard deviation of the cell values. */
    double stdDev;

    /** \brief Collected statistics */
    int statsGathered;

    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sum;

    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumOfSquares;

    /** \brief Number of columns used to calc statistics */
    int width;

    /** \brief Number of rows used to calc statistics */
    int height;

    /** \brief Extent used to calc statistics */
    QgsRectangle extent;
};

