class QgsNetworkSpeedStrategy : QgsNetworkStrategy
{
%TypeHeaderCode
#include <qgsnetworkspeedstrategy.h>
%End

  public:
    QgsNetworkSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor );

    QVariant cost( double distance, const QgsFeature& f ) const;

    QgsAttributeList requiredAttributes() const;
};
