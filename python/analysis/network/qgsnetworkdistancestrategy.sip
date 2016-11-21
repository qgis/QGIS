class QgsNetworkDistanceStrategy : QgsNetworkStrategy
{
%TypeHeaderCode
#include <qgsnetworkdistancestrategy.h>
%End

  public:
    virtual QVariant cost( double distance, const QgsFeature& ) const;
};
