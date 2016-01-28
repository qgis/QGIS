
#ifndef QGSDB2NEWCONNECTION_H
#define QGSDB2NEWCONNECTION_H
#include "ui_qgsdb2newconnectionbase.h"
#include "qgisgui.h"
#include "qgscontexthelp.h"

static const int
ENV_LUW = 1,
          ENV_ZOS = 2;

/** \class QgsDb2NewConnection
 * \brief Dialog to allow the user to configure and save connection
 * information for an DB2 database
 */
class QgsDb2NewConnection : public QDialog, private Ui::QgsDb2NewConnectionBase
{
    Q_OBJECT
  public:
    //! Constructor
    QgsDb2NewConnection( QWidget *parent = 0, const QString& connName = QString::null, Qt::WindowFlags fl = QgisGui::ModalDialogFlags );

    //! Destructor
    ~QgsDb2NewConnection();

    //! Tests the connection using the parameters supplied
    bool testConnection( QString testDatabase = QString() );
    /**
     * @brief List all databases found for the given server.
     */
    void listDatabases();
  public slots:
    void accept() override;
    void on_btnListDatabase_clicked();
    void on_btnConnect_clicked();
    void on_cb_trustedConnection_clicked();
    void on_buttonBox_helpRequested() { QgsContextHelp::run( metaObject()->className() ); }
  private:
    QString mOriginalConnName; //store initial name to delete entry in case of rename
};

#endif //  QGSDB2NEWCONNECTION_H
