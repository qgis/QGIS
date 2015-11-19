/** \ingroup core
 * \class QgsFeatureFilterProvider
 * Abstract interface for use by classes that filter the features of a layer.
 * A QgsFeatureFilterProvider provides a method for modifying a QgsFeatureRequest in place to apply
 * additional filters to the request.
 * \note added in QGIS 2.14
 **/
class QgsFeatureFilterProvider
{
%TypeHeaderCode
#include <qgsfeaturefilterprovider.h>
%End

  public:

    /** Add additional filters to the feature request to further restrict the features returned by the request.
     * Derived classes must implement this method.
     * @param layer the layer to filter
     * @param featureRequest the feature request to update
     */
    virtual void filterFeatures( const QgsVectorLayer* layer, QgsFeatureRequest& featureRequest ) const = 0;

    /** Create a clone of the feature filter provider
     * @return a new clone
     */
    virtual QgsFeatureFilterProvider* clone() const = 0 /Factory/;
};
