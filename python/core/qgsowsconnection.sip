class QgsOwsConnection : QObject
{
%TypeHeaderCode
#include <qgsowsconnection.h>
%End

  public:
    /**
     * Constructor
     * @param service service name: WMS,WFS,WCS
     * @param connName connection name
     */
    QgsOwsConnection( const QString & service, const QString & connName );

    //! Destructor
    ~QgsOwsConnection();

    /** Returns the list of connections for the specified service */
    static QStringList connectionList( const QString & service );

    /** Deletes the connection for the specified service with the specified name */
    static void deleteConnection( const QString & service, const QString & name );

    /** Retrieves the selected connection for the specified service */
    static QString selectedConnection( const QString & service );
    /** Marks the specified connection for the specified service as selected */
    static void setSelectedConnection( const QString & service, const QString & name );

    QString mConnName;
    /** Returns the connection uri */
    QgsDataSourceUri uri() const;
    QString mConnectionInfo;

};
