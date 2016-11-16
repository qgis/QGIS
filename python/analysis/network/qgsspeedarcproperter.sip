class QgsSpeedArcProperter : QgsArcProperter
{
%TypeHeaderCode
#include <qgsspeedarcproperter.h>
%End

  public:
    QgsSpeedArcProperter( int attributeId, double defaultValue, double toMetricFactor );

    QVariant property( double distance, const QgsFeature& f ) const;

    QgsAttributeList requiredAttributes() const;
};
