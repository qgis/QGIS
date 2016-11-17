class QgsSpeedStrategy : QgsStrategy
{
%TypeHeaderCode
#include <qgsspeedstrategy.h>
%End

  public:
    QgsSpeedStrategy( int attributeId, double defaultValue, double toMetricFactor );

    QVariant cost( double distance, const QgsFeature& f ) const;

    QgsAttributeList requiredAttributes() const;
};
