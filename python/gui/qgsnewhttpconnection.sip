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

    QString name() const;
%Docstring
 Returns the current connection name.
.. versionadded:: 3.0
 :rtype: str
%End

    QString url() const;
%Docstring
 Returns the current connection url.
.. versionadded:: 3.0
 :rtype: str
%End

  public slots:

    virtual void accept();


  protected:

    virtual bool validate();
%Docstring
 Returns true if dialog settings are valid, or false if current
 settings are not valid and the dialog should not be acceptable.
.. versionadded:: 3.0
 :rtype: bool
%End

    QPushButton *testConnectButton();
%Docstring
 Returns the "test connection" button.
.. versionadded:: 3.0
 :rtype: QPushButton
%End

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
