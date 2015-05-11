
class QgsDataItemProviderRegistry
{
%TypeHeaderCode
#include <qgsdataitemproviderregistry.h>
%End
  public:
    //! Returns the instance pointer, creating the object on the first call
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

};
