#ifndef QGSDATASOURCEMANAGERDIALOG_H
#define QGSDATASOURCEMANAGERDIALOG_H

#include <QList>
#include <QDialog>
class QgsBrowserDockWidget;


namespace Ui
{
  class QgsDataSourceManagerDialog;
}

class QgsDataSourceManagerDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit QgsDataSourceManagerDialog( QWidget *parent = 0 );
    ~QgsDataSourceManagerDialog();

  public slots:
    void setCurrentPage( int index );

  private:
    Ui::QgsDataSourceManagerDialog *ui;
    QgsBrowserDockWidget *mBrowserWidget = nullptr;
    QList<QDialog *> mDialogs;
};

#endif // QGSDATASOURCEMANAGERDIALOG_H
