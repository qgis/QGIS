/** \ingroup analysis
 *
 *
 */

class QgsTransectSample
{
%TypeHeaderCode
#include <qgstransectsample.h>
%End
  public:

    enum DistanceUnits
    {
      Meters,
      StrataUnits //units are the same as stratum layer
    };

    QgsTransectSample( QgsVectorLayer* strataLayer, const QString& strataIdAttribute, const QString& minDistanceAttribute, const QString& nPointsAttribute,
                       DistanceUnits minDistUnits, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       const QString& baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer, const QString& usedBaselineLayer,
                       double minTransectLength = 0.0, double baselineBufferDistance = -1.0, double baselineSimplificationTolerance = -1.0 );

    int createSample( QProgressDialog* pd );
};
