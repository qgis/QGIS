#ifndef QGSDATADEFINEDSYMBOLLAYERDIALOG_H
#define QGSDATADEFINEDSYMBOLLAYERDIALOG_H

#include "ui_qgsdatadefinedsymboldialogbase.h"
#include <QDialog>

class QgsVectorLayer;

class QgsDataDefinedSymbolDialog: public QDialog, private Ui::QgsDataDefinedSymbolDialog
{
    Q_OBJECT
  public:
    QgsDataDefinedSymbolDialog( const QMap< QString, QPair< QString, QString > >& properties, const QgsVectorLayer* vl, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDataDefinedSymbolDialog();
    QMap< QString, QString > dataDefinedProperties() const;

  private slots:
    void expressionButtonClicked();

  private:
    const QgsVectorLayer* mVectorLayer;
};

#endif // QGSDATADEFINEDSYMBOLLAYERDIALOG_H
