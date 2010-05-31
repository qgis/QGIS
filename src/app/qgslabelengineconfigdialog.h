#ifndef QGSLABELENGINECONFIGDIALOG_H
#define QGSLABELENGINECONFIGDIALOG_H

#include <QDialog>

#include "ui_qgsengineconfigdialog.h"

class QgsPalLabeling;

class QgsLabelEngineConfigDialog : public QDialog, private Ui::QgsEngineConfigDialog
{
    Q_OBJECT
  public:
    QgsLabelEngineConfigDialog( QgsPalLabeling* lbl, QWidget* parent = NULL );

  public slots:
    void onOK();

  protected:
    QgsPalLabeling* mLBL;
};

#endif // QGSLABELENGINECONFIGDIALOG_H
