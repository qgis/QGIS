#ifndef QGSPENDINGCHANGESWIDGET_H
#define QGSPENDINGCHANGESWIDGET_H

#include <QObject>
#include <QWidget>

#include "qgis_gui.h"
#include "qgspendingchangesmodel.h"
#include "ui_qgspendingchangesbase.h"


class GUI_EXPORT QgsPendingChangesWidget : public QWidget, private Ui::QgsPendindChangesBase
{
    Q_OBJECT
  public:
    explicit QgsPendingChangesWidget( QWidget *parent = 0 );

    void setModel( QgsPendingChangesModel *model );

  signals:

  public slots:
};

#endif // QGSPENDINGCHANGESWIDGET_H
