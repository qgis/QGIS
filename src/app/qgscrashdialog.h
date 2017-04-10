#ifndef QGSCRASHDIALOG_H
#define QGSCRASHDIALOG_H

#include <QDialog>
#include "ui_qgscrashdialog.h"
#include "qgis_app.h"

class APP_EXPORT QgsCrashDialog : public QDialog, private Ui::QgsCrashDialog
{
    Q_OBJECT
  public:
    QgsCrashDialog();
};

#endif // QGSCRASHDIALOG_H
