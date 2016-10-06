
/**
 * This singleton class keeps a list of data item providers that may add items to the browser tree.
 * When created, it automatically adds providers from provider plugins (e.g. PostGIS, WMS, ...)
 *
 * @note added in 2.10
 */
class QgsDataItemProviderRegistry
{
%TypeHeaderCode
#include <qgsdataitemproviderregistry.h>
%End
  public:
    static QgsDataItemProviderRegistry * instance();

    ~QgsDataItemProviderRegistry();

    //! Get list of available providers
    QList<QgsDataItemProvider*> providers() const;

    //! Add a provider implementation. Takes ownership of the object.
    void addProvider( QgsDataItemProvider* provider /Transfer/ );

    //! Remove provider implementation from the list (provider object is deleted)
    void removeProvider( QgsDataItemProvider* provider );

  private:
    QgsDataItemProviderRegistry();
    QgsDataItemProviderRegistry( const QgsDataItemProviderRegistry& rh );

};
