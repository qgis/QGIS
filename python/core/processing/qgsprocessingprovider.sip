/**
 * \class QgsProcessingProvider
 * \ingroup core
 * Abstract base class for processing providers. An algorithm provider is a set of
 * related algorithms, typically from the same external application or related
 * to a common area of analysis.
 * \note added in QGIS 3.0
 */
class QgsProcessingProvider
{
%TypeHeaderCode
#include <qgsprocessingprovider.h>
%End

  public:

    QgsProcessingProvider();

    virtual ~QgsProcessingProvider();

    /**
     * Returns an icon for the provider.
     */
    virtual QIcon icon() const;

    /**
     * Returns the unique provider id, used for identifying the provider. This string
     * should be a unique, short, character only string, eg "qgis" or "gdal". This
     * string should not be localised.
     * @see name()
     */
    virtual QString id() const = 0;

    /**
     * Returns the full provider name, which is used to describe the provider within the GUI.
     * This string should be localised.
     * @see id()
     */
    virtual QString name() const = 0;

    virtual bool canBeActivated() const;

  private:

    //! Providers cannot be copied
    QgsProcessingProvider( const QgsProcessingProvider& other );
    //! Providers cannot be copied
    //QgsProcessingProvider& operator=( const QgsProcessingProvider& other );
};
