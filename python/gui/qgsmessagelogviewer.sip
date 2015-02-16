/** \ingroup gui
 * A generic dialog widget for displaying QGIS log messages.
 */
class QgsMessageLogViewer: QDialog
{
%TypeHeaderCode
#include <qgsmessagelogviewer.h>
%End

  public:
    QgsMessageLogViewer( QStatusBar *statusBar = 0, QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsMessageLogViewer();

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );
};
