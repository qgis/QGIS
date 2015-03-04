
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

  private:
    QgsDataItemProviderRegistry();

};
