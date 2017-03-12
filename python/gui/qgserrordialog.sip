class QgsErrorDialog : QDialog
{
%TypeHeaderCode
#include <qgserrordialog.h>
%End
  public:
    QgsErrorDialog( const QgsError & error, const QString & title, QWidget *parent /TransferThis/ = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsErrorDialog();

    /** Show dialog with error
     * @param error error
     * @param title title
     * @param parent parent object
     * @param fl widget flags
     */
    static void show( const QgsError & error, const QString & title, QWidget *parent = 0, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );

  public slots:
    void on_mDetailPushButton_clicked();
    void on_mDetailCheckBox_stateChanged( int state );
};
