
/** \ingroup core
 * \class QgsVectorLayerUtils
 * \brief Contains utility methods for working with QgsVectorLayers.
 *
 * \note Added in version 3.0
 */
class QgsVectorLayerUtils
{
%TypeHeaderCode
#include <qgsvectorlayerutils.h>
%End
  public:

    /**
     * Returns true if the specified value already exists within a field. This method can be used to test for uniqueness
     * of values inside a layer's attributes. An optional list of ignored feature IDs can be provided, if so, any features
     * with IDs within this list are ignored when testing for existance of the value.
     */
    static bool valueExists( const QgsVectorLayer* layer, int fieldIndex, const QVariant& value, const QgsFeatureIds& ignoreIds = QgsFeatureIds() );

};
