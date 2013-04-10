typedef QList<QgsRasterRange> QgsRasterRangeList;

class QgsRasterRange
{
%TypeHeaderCode
#include <qgsrasterrange.h>
%End

  public:
    QgsRasterRange();
    
    QgsRasterRange( double theMin, double theMax  );
    
    ~QgsRasterRange();
    
    double min() const;
    double max() const;
    
    double setMin( double theMin );
    double setMax( double theMax );
    
    bool operator==( const QgsRasterRange &o ) const;

    bool contains( double value, const QgsRasterRangeList &rangeList );
};
