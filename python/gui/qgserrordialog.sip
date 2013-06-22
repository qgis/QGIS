class QgsErrorDialog : public QDialog
{
%TypeHeaderCode
#include <qgserrordialog.h>
%End
  public:
    QgsErrorDialog( const QgsError & theError, const QString & theTitle, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsErrorDialog();

    /** Show dialog with error
     * @param theError error
     * @param theTitle title
     * @param parent parent object
     * @param fl widget flags
     */
    static void show( const QgsError & theError, const QString & theTitle, QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );

  public slots:
    void on_mDetailPushButton_clicked();
    void on_mDetailCheckBox_stateChanged( int state );
};
