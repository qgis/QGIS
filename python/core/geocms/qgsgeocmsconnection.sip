/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/qgsgeocmsconnection.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsGeoCmsConnection : QObject
{
%Docstring
 Base class for all GeoCMS connection.
 Parent/children hierarchy is not based on QObject.
%End

%TypeHeaderCode
#include "qgsgeocmsconnection.h"
%End
  public:

    QgsGeoCmsConnection( const QString &geoCMSName, const QString &connName );
%Docstring
 Constructor
 \param geoCMSName GeoCMS name eg: geonode, qgiscloud, etc
 \param connName connection name
%End

    ~QgsGeoCmsConnection();
%Docstring
Destructor
%End


    QString geoCMSName() const;
%Docstring
 :rtype: str
%End
    void setGeoCMSName( const QString &geoCMSName );

    QString connName() const;
%Docstring
 :rtype: str
%End
    void setConnName( const QString &connName );

    QgsDataSourceUri uri();
%Docstring
 :rtype: QgsDataSourceUri
%End
    void setUri( const QgsDataSourceUri &uri );

    static QStringList connectionList( const QString &geoCMSName );
%Docstring
Returns the list of connections for the specified GeoCMS
 :rtype: list of str
%End

    static void deleteConnection( const QString &geoCMSName, const QString &name );
%Docstring
Deletes the connection for the specified GeoCMS with the specified name
%End

    static QString selectedConnection( const QString &geoCMSName );
%Docstring
Retrieves the selected connection for the specified GeoCMS
 :rtype: str
%End

    static void setSelectedConnection( const QString &geoCMSName, const QString &name );
%Docstring
Marks the specified connection for the specified GeoCMS as selected
%End

};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/geocms/qgsgeocmsconnection.h                                *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
