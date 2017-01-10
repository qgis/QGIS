
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
     * @see createUniqueValue()
     */
    static bool valueExists( const QgsVectorLayer* layer, int fieldIndex, const QVariant& value, const QgsFeatureIds& ignoreIds = QgsFeatureIds() );

    /**
     * Returns a new attribute value for the specified field index which is guaranteed to be unique. The optional seed
     * value can be used as a basis for generated values.
     * @see valueExists()
     */
    static QVariant createUniqueValue( const QgsVectorLayer* layer, int fieldIndex, const QVariant& seed = QVariant() );

    /**
     * Tests an attribute value to check whether it passes all constraints which are present on the corresponding field.
     * Returns true if the attribute value is valid for the field. Any constraint failures will be reported in the errors argument.
     * If the strength or origin parameter is set then only constraints with a matching strength/origin will be checked.
     */
    static bool validateAttribute( const QgsVectorLayer* layer, const QgsFeature& feature, int attributeIndex, QStringList& errors /Out/,
                                   QgsFieldConstraints::ConstraintStrength strength = QgsFieldConstraints::ConstraintStrengthNotSet,
                                   QgsFieldConstraints::ConstraintOrigin origin = QgsFieldConstraints::ConstraintOriginNotSet );

    /**
     * Creates a new feature ready for insertion into a layer. Default values and constraints
     * (e.g., unique constraints) will automatically be handled. An optional attribute map can be
     * passed for the new feature to copy as many attribute values as possible from the map,
     * assuming that they respect the layer's constraints. Note that the created feature is not
     * automatically inserted into the layer.
     */
    static QgsFeature createFeature( QgsVectorLayer* layer,
                                     const QgsGeometry& geometry = QgsGeometry(),
                                     const QgsAttributeMap& attributes = QgsAttributeMap(),
                                     QgsExpressionContext* context = nullptr );

};
