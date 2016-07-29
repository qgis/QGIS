/** \ingroup analysis
 */

class QgsPointSample
{
%TypeHeaderCode
#include <qgspointsample.h>
%End
  public:
    QgsPointSample( QgsVectorLayer* inputLayer, const QString& outputLayer, const QString& nPointsAttribute, const QString& minDistAttribute = QString() );

    /** Starts calculation of random points
        @return 0 in case of success*/
    int createRandomPoints( QProgressDialog* pd );
};
