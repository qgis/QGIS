/**
 * Interface used by class that will filter the features of a layer.
 * The only method `filterFeatures` fill the `QgsFeatureRequest` to get only the
 * wanted features.
 **/
class QgsFeatureFilterProvider
{
%TypeHeaderCode
#include <qgsfeaturefilterprovider.h>
%End

  public:
    /** Add some filter to the feature request to don't have the unauthorized (unauthorised) features
     * @param layer the layer to filter
     * @param featureRequest the feature request to update
     * @note not available in Python bindings
     */
    virtual void filterFeatures( const QgsVectorLayer* layer, QgsFeatureRequest& featureRequest ) const = 0;

    /** Create a clone of the feature filter provider
     * @return a new clone
     */
    virtual QgsFeatureFilterProvider* clone() const = 0;
};
