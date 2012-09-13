
/** \brief The RasterBandStats struct is a container for statistics about a single
 * raster band.
 */
class QgsRasterBandStats
{
%TypeHeaderCode
#include <qgsrasterbandstats.h>
%End

  public:
    /** \brief The name of the band that these stats belong to. */
    QString bandName;
    /** \brief The gdal band number (starts at 1)*/
    int bandNumber; 
    /** \brief Collected statistics */
    int statsGathered; 
    /** \brief The minimum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMinimum function. */
    double minimumValue;
    /** \brief The maximum cell value in the raster band. NO_DATA values
     * are ignored. This does not use the gdal GetMaximmum function. */
    double maximumValue;
    /** \brief The range is the distance between min & max. */
    double range;
    /** \brief The mean cell value for the band. NO_DATA values are excluded. */
    double mean;
    /** \brief The sum of the squares. Used to calculate standard deviation. */
    double sumOfSquares; 
    /** \brief The standard deviation of the cell values. */
    double stdDev;
    /** \brief The sum of all cells in the band. NO_DATA values are excluded. */
    double sum;
    /** \brief The number of cells in the band. Equivalent to height x width. 
     * TODO: check if NO_DATA are excluded!*/
    int elementCount;    
    /** Color table */
    //QList<QgsColorRampShader::ColorRampItem> colorTable;
};

