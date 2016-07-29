/**
 * Utils class for QgsVirtualLayerDefinition
 */
class QgsVirtualLayerDefinitionUtils
{
%TypeHeaderCode
#include <qgsvirtuallayerdefinitionutils.h>
%End
  public:
    //! Get a virtual layer definition from a vector layer where vector joins are replaced by SQL LEFT JOINs
    static QgsVirtualLayerDefinition fromJoinedLayer( QgsVectorLayer* joinedLayer );
};
