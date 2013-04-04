
class QgsDistanceArcProperter : QgsArcProperter
{
%TypeHeaderCode
#include <qgsdistancearcproperter.h>
%End

  public:
    virtual QVariant property( double distance, const QgsFeature& ) const;
};

