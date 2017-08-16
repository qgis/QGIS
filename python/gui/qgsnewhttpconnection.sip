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
    QgsNewHttpConnection( QWidget *parent /TransferThis/ = 0, const QString &baseKey = "qgis/connections-wms/", const QString &connName = QString(), Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
%Docstring
Constructor
%End

  public slots:
    virtual void accept();

    void on_txtName_textChanged( const QString & );
    void on_txtUrl_textChanged( const QString & );

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsnewhttpconnection.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
