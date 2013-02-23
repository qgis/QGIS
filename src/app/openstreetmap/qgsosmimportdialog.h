#ifndef QGSOSMIMPORTDIALOG_H
#define QGSOSMIMPORTDIALOG_H

#include <QDialog>

#include "ui_qgsosmimportdialog.h"

class QgsOSMXmlImport;

class QgsOSMImportDialog : public QDialog, private Ui::QgsOSMImportDialog
{
    Q_OBJECT
  public:
    explicit QgsOSMImportDialog( QWidget* parent = 0 );
    ~QgsOSMImportDialog();

  private slots:
    void onBrowseXml();
    void onBrowseDb();

    void xmlFileNameChanged( const QString& fileName );
    void dbFileNameChanged( const QString& fileName );

    void onOK();
    void onClose();

    void onProgress( int percent );

  private:
    QgsOSMXmlImport* mImport;
};

#endif // QGSOSMIMPORTDIALOG_H
