class QgsOwsConnection : QObject
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
    QgsOwsConnection( const QString & theService, const QString & theConnName );

    //! Destructor
    ~QgsOwsConnection();

    /** Returns the list of connections for the specified service */
    static QStringList connectionList( const QString & theService );

    /** Deletes the connection for the specified service with the specified name */
    static void deleteConnection( const QString & theService, const QString & name );

    /** Retreives the selected connection for the specified service */
    static QString selectedConnection( const QString & theService );
    /** Marks the specified connection for the specified service as selected */
    static void setSelectedConnection( const QString & theService, const QString & name );

    QString mConnName;
    /** Returns the connection uri */
    QgsDataSourceUri uri() const;
    QString mConnectionInfo;

};
