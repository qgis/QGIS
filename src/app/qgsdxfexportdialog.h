#ifndef QGSDXFEXPORTDIALOG_H
#define QGSDXFEXPORTDIALOG_H

#include "ui_qgsdxfexportdialogbase.h"
#include "qgsdxfexport.h"

class QgsDxfExportDialog: public QDialog, private Ui::QgsDxfExportDialogBase
{
    Q_OBJECT
  public:
    QgsDxfExportDialog( const QList<QString>& layerKeys, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDxfExportDialog();

    QList<QString> layers() const;
    double symbologyScale() const;
    QgsDxfExport::SymbologyExport symbologyMode() const;
    QString saveFile() const;

  private slots:
    void on_mFileSelectionButton_clicked();
    void setOkEnabled();
    void saveSettings();
};

#endif // QGSDXFEXPORTDIALOG_H
