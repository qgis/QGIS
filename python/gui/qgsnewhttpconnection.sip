class QgsNewHttpConnection : QDialog
{
%TypeHeaderCode
#include <qgsnewhttpconnection.h>
%End

  public:
    //! Constructor
    QgsNewHttpConnection( QWidget *parent /TransferThis/ = 0, const QString& baseKey = "/Qgis/connections-wms/", const QString& connName = QString::null, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );
    //! Destructor
    ~QgsNewHttpConnection();
  public slots:
    //! Saves the connection to ~/.qt/qgisrc
    void accept();

    void on_txtName_textChanged( const QString & );

    void on_txtUrl_textChanged( const QString & );

    void on_buttonBox_helpRequested();
};
