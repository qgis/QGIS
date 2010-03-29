#ifndef QGSFORMANNOTATIONDIALOG_H
#define QGSFORMANNOTATIONDIALOG_H

#include "ui_qgsformannotationdialogbase.h"
#include "qgsformannotationitem.h"

class QgsAnnotationWidget;

class GUI_EXPORT QgsFormAnnotationDialog: public QDialog, private Ui::QgsFormAnnotationDialogBase
{
    Q_OBJECT
  public:
    QgsFormAnnotationDialog( QgsFormAnnotationItem* item, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsFormAnnotationDialog();

  private:
    QgsFormAnnotationItem* mItem;
    QgsAnnotationWidget* mEmbeddedWidget;

  private slots:
    void applySettingsToItem();
    void on_mBrowseToolButton_clicked();
    void deleteItem();
};

#endif // QGSFORMANNOTATIONDIALOG_H
