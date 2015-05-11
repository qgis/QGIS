
class QgsDataItemProvider
{
%TypeHeaderCode
#include <qgsdataitemprovider.h>
%End
  public:
    virtual ~QgsDataItemProvider();

    //! Human-readable name of the provider name
    virtual QString name() = 0;

    //! Return combination of flags from QgsDataProvider::DataCapabilities
    virtual int capabilities() = 0;

    //! Create a new instance of QgsDataItem (or null) for given path and parent item.
    //! Caller takes responsibility of deleting created items.
    virtual QgsDataItem* createDataItem( const QString& path, QgsDataItem* parentItem ) = 0 /Factory/;

};
