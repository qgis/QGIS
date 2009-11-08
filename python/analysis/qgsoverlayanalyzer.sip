/** polyline is just a list of points */
typedef QMap<int, QgsField> QgsFieldMap;

/** \ingroup analysis
 * The QGis class provides vector geometry analysis functions
 */

class QgsOverlayAnalyzer
{
%TypeHeaderCode
#include <qgsoverlayanalyzer.h>
%End

  public:

    /**Perform an intersection on two input vector layers and write output to a new shape file
    */
    bool intersection( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
                       const QString& shapefileName, bool onlySelectedFeatures = false,
                       QProgressDialog* p = 0 );

  private:
    void combineFieldLists( QgsFieldMap fieldListA, QgsFieldMap fieldListB );
    void intersectFeature( QgsFeature& f, QgsVectorFileWriter* vfw, QgsVectorLayer* dp,
                           QgsSpatialIndex* index );
};
