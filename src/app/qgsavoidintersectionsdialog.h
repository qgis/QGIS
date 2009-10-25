#ifndef QGSAVOIDINTERSECTIONSDIALOG_H
#define QGSAVOIDINTERSECTIONSDIALOG_H

#include "ui_qgsavoidintersectionsdialogbase.h"

class QgsMapCanvas;

class QgsAvoidIntersectionsDialog: public QDialog, private Ui::QgsAvoidIntersectionsDialogBase
{
    Q_OBJECT
  public:
    QgsAvoidIntersectionsDialog( QgsMapCanvas* canvas, const QSet<QString>& enabledLayers, QWidget * parent = 0, Qt::WindowFlags f = 0 );
    ~QgsAvoidIntersectionsDialog();
    /**Returns ids of layers that are considered for the avoid intersection function*/
    void enabledLayers( QSet<QString>& enabledLayers );

  private:
    QgsMapCanvas* mMapCanvas;
};

#endif // QGSAVOIDINTERSECTIONSDIALOG_H
