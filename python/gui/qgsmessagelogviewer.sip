/** \ingroup gui
 * A generic message for displaying QGIS log messages.
 * \note added in 1.8
 */
class QgsMessageLogViewer: QDialog
{
%TypeHeaderCode
#include <qgsmessagelogviewer.h>
%End

  public:
    QgsMessageLogViewer( QStatusBar *statusBar = 0, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsMessageLogViewer();

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );
};
