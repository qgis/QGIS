#ifndef QGSOSMEXPORTDIALOG_H
#define QGSOSMEXPORTDIALOG_H

#include <QDialog>

#include "ui_qgsosmexportdialog.h"

class QgsOSMDatabase;

class QStandardItemModel;

class QgsOSMExportDialog : public QDialog, private Ui::QgsOSMExportDialog
{
    Q_OBJECT
  public:
    explicit QgsOSMExportDialog( QWidget *parent = 0 );
    ~QgsOSMExportDialog();

  protected:
    bool openDatabase();

  private slots:
    void onBrowse();
    void updateLayerName();
    void onLoadTags();

    void onOK();
    void onClose();

  private:
    QgsOSMDatabase* mDatabase;
    QStandardItemModel* mTagsModel;
};

#endif // QGSOSMEXPORTDIALOG_H
