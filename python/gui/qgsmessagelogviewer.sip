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
    QgsMessageLogViewer( QStatusBar *statusBar = 0, QWidget *parent = 0, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    ~QgsMessageLogViewer();

    //! @note added in 2.4
    void setShowToolTips( bool enabled );
    //! @note added in 2.4
    bool showToolTips() const;

  public slots:
    void logMessage( QString message, QString tag, QgsMessageLog::MessageLevel level );

  private:
    void showEvent( QShowEvent * );
    void hideEvent( QHideEvent * );
};
