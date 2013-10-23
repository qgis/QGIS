#ifndef QGSDATUMTRANSFORMDIALOG_H
#define QGSDATUMTRANSFORMDIALOG_H

#include "ui_qgsdatumtransformdialogbase.h"

class QgsDatumTransformDialog: public QDialog, private Ui::QgsDatumTransformDialogBase
{
  public:
    QgsDatumTransformDialog( const QString& layerName, const QList< QList< int > >& dt, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsDatumTransformDialog();

    QList< int > selectedDatumTransform();
  private:
    QgsDatumTransformDialog();
};

#endif // QGSDATUMTRANSFORMDIALOG_H
