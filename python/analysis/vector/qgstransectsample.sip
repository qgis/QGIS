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

    QgsTransectSample( QgsVectorLayer* strataLayer, QString strataIdAttribute, QString minDistanceAttribute, QString nPointsAttribute,
                       DistanceUnits minDistUnits, QgsVectorLayer* baselineLayer, bool shareBaseline,
                       QString baselineStrataId, const QString& outputPointLayer, const QString& outputLineLayer, const QString& usedBaselineLayer, double minTransectLength = 0.0 );
    ~QgsTransectSample();

    int createSample( QProgressDialog* pd );
};
