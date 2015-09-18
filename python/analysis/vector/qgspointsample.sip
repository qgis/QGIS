/** \ingroup analysis
 */

class QgsPointSample
{
%TypeHeaderCode
#include <qgspointsample.h>
%End
  public:
    QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, QString nPointsAttribute, QString minDistAttribute = QString() );
    ~QgsPointSample();

    /** Starts calculation of random points
        @return 0 in case of success*/
    int createRandomPoints( QProgressDialog* pd );
};
