/** \ingroup gui
 * Abstract base class that may be implemented to handle new types of data to be dropped in QGIS.
 * Implementations will be used when a QgsMimeDataUtils::Uri has layerType equal to "custom",
 * and the providerKey is equal to key() returned by the implementation.
 * @note added in QGIS 3.0
 */
class QgsCustomDropHandler
{
%TypeHeaderCode
#include <qgscustomdrophandler.h>
%End

  public:
    virtual ~QgsCustomDropHandler();

    //! Type of custom URI recognized by the handler
    virtual QString key() const = 0;

    //! Method called from QGIS after a drop event with custom URI known by the handler
    virtual void handleDrop( const QgsMimeDataUtils::Uri& uri ) const = 0;
};
