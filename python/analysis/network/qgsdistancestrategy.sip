class QgsDistanceStrategy : QgsStrategy
{
%TypeHeaderCode
#include <qgsdistancestrategy.h>
%End

  public:
    virtual QVariant cost( double distance, const QgsFeature& ) const;
};
