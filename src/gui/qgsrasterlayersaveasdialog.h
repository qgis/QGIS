#ifndef QGSRASTERLAYERSAVEASDIALOG_H
#define QGSRASTERLAYERSAVEASDIALOG_H

#include "ui_qgsrasterlayersaveasdialogbase.h"

class QgsRasterDataProvider;

class QgsRasterLayerSaveAsDialog: public QDialog, private Ui::QgsRasterLayerSaveAsDialogBase
{
  public:
    QgsRasterLayerSaveAsDialog( QgsRasterDataProvider* sourceProvider, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterLayerSaveAsDialog();

    int nColumns() const;
    int nRows() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;
    QString outputFileName() const;
    QString outputFormat() const;

  private:
    QgsRasterDataProvider* mDataProvider;

    void setValidators();
};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
