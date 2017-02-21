typedef QList<QgsRasterRange> QgsRasterRangeList;

/** \ingroup core
 * Raster values range container. Represents range of values between min and max
 * including min and max value.
 */
class QgsRasterRange
{
%TypeHeaderCode
#include <qgsrasterrange.h>
%End

  public:
    /** \brief Constructor.
     */
    QgsRasterRange();

    /** \brief Constructor
     *  @param min minimum value
     *  @param max max value
     */
    QgsRasterRange( double min, double max );

    double min() const;
    double max() const;

    double setMin( double min );
    double setMax( double max );

    bool operator==( QgsRasterRange o ) const;

    /** \brief Test if value is within the list of ranges
     *  @param value value
     *  @param rangeList list of ranges
     *  @return true if value is in at least one of ranges
     *  @note not available in python bindings
     */
    static bool contains( double value, const QgsRasterRangeList &rangeList );
};
