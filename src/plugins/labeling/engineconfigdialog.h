#ifndef ENGINECONFIGDIALOG_H
#define ENGINECONFIGDIALOG_H

#include <QDialog>

#include "ui_engineconfigdialog.h"

class PalLabeling;

class EngineConfigDialog : public QDialog, private Ui::EngineConfigDialog
{
    Q_OBJECT
  public:
    EngineConfigDialog( PalLabeling* lbl, QWidget* parent = NULL );

  public slots:
    void onOK();

  protected:
    PalLabeling* mLBL;
};

#endif // ENGINECONFIGDIALOG_H
