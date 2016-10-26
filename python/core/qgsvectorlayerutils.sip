
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

    /**
     * Tests an attribute value to check whether it passes all constraints which are present on the corresponding field.
     * Returns true if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
     */
    static bool validateAttribute( const QgsVectorLayer* layer, const QgsFeature& feature, int attributeIndex, QStringList& errors /Out/ );

};
