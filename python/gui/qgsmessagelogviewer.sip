/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagelogviewer.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/





class QgsMessageLogViewer: QDialog
{
%Docstring
 A generic dialog widget for displaying QGIS log messages.
%End

%TypeHeaderCode
#include "qgsmessagelogviewer.h"
%End
  public:
    QgsMessageLogViewer( QStatusBar *statusBar = 0, QWidget *parent /TransferThis/ = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );

  protected:
    void closeEvent( QCloseEvent *e );
    void reject();

};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsmessagelogviewer.h                                        *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
