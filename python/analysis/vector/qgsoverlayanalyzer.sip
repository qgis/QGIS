
/** \ingroup analysis
 * The QGis class provides vector overlay analysis functions
 */

class QgsOverlayAnalyzer
{
%TypeHeaderCode
#include <qgsoverlayanalyzer.h>
%End

  public:

    /** Perform an intersection on two input vector layers and write output to a new shape file
      @param layerA input vector layer
      @param layerB input vector layer
      @param shapefileName path to the output shp
      @param onlySelectedFeatures if true, only selected features are considered, else all the features
      @param p progress dialog (or 0 if no progress dialog is to be shown)
      */
    bool intersection( QgsVectorLayer* layerA, QgsVectorLayer* layerB,
                       const QString& shapefileName, bool onlySelectedFeatures = false,
                       QProgressDialog* p = 0 );
};
