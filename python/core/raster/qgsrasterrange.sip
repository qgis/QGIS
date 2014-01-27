typedef QList<QgsRasterRange> QgsRasterRangeList;

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
     *  @param theMin minimum value
     *  @param theMax max value
     */
    QgsRasterRange( double theMin, double theMax );

    ~QgsRasterRange();

    double min() const;
    double max() const;

    double setMin( double theMin );
    double setMax( double theMax );

    bool operator==( const QgsRasterRange &o ) const;

    /** \brief Test if value is within the list of ranges
     *  @param value value
     *  @param rangeList list of ranges
     *  @return true if value is in at least one of ranges
     *  @note not available in python bindings
     */
    static bool contains( double value, const QgsRasterRangeList &rangeList );
};
