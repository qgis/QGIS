class QgsOWSConnection : QObject
{
%TypeHeaderCode
#include <qgsowsconnection.h>
%End

  public:
    /**
     * Constructor
     * @param theService service name: WMS,WFS,WCS
     * @param theConnName connection name
     */
    QgsOWSConnection( const QString & theService, const QString & theConnName );

    //! Destructor
    ~QgsOWSConnection();

    static QStringList connectionList( const QString & theService );

    static void deleteConnection( const QString & theService, const QString & name );

    static QString selectedConnection( const QString & theService );
    static void setSelectedConnection( const QString & theService, const QString & name );

    QString mConnName;
    QgsDataSourceURI uri();
    QString mConnectionInfo;

    QString connectionInfo() /Deprecated/;
};
