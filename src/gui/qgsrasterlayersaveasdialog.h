#ifndef QGSRASTERLAYERSAVEASDIALOG_H
#define QGSRASTERLAYERSAVEASDIALOG_H

#include "ui_qgsrasterlayersaveasdialogbase.h"

class QgsRasterDataProvider;

class QgsRasterLayerSaveAsDialog: public QDialog, private Ui::QgsRasterLayerSaveAsDialogBase
{
    Q_OBJECT
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

  private slots:
    void on_mBrowseButton_clicked();
    void on_mSaveAsLineEdit_textChanged( const QString& text );

  private:
    QgsRasterDataProvider* mDataProvider;

    void setValidators();
};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
