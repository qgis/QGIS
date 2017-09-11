/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewhttpconnection.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsNewHttpConnection : QDialog
{
%Docstring
 Dialog to allow the user to configure and save connection
 information for an HTTP Server for WMS, etc.
%End

%TypeHeaderCode
#include "qgsnewhttpconnection.h"
%End
  public:

    enum ConnectionType
    {
      ConnectionWfs,
      ConnectionWms,
      ConnectionWcs,
      ConnectionOther,
    };
    typedef QFlags<QgsNewHttpConnection::ConnectionType> ConnectionTypes;


    enum Flag
    {
      FlagShowTestConnection,
    };
    typedef QFlags<QgsNewHttpConnection::Flag> Flags;


    QgsNewHttpConnection( QWidget *parent /TransferThis/ = 0,
                          QgsNewHttpConnection::ConnectionTypes types = ConnectionWms,
                          const QString &baseKey = "qgis/connections-wms/",
                          const QString &connectionName = QString(),
                          QgsNewHttpConnection::Flags flags = 0,
                          Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
 Constructor for QgsNewHttpConnection.

 The ``types`` argument dictates which connection type settings should be
 shown in the dialog.

 The ``flags`` argument allows specifying flags which control the dialog behavior
 and appearance.
%End

  public slots:

    virtual void accept();


};

QFlags<QgsNewHttpConnection::ConnectionType> operator|(QgsNewHttpConnection::ConnectionType f1, QFlags<QgsNewHttpConnection::ConnectionType> f2);

QFlags<QgsNewHttpConnection::Flag> operator|(QgsNewHttpConnection::Flag f1, QFlags<QgsNewHttpConnection::Flag> f2);


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewhttpconnection.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
