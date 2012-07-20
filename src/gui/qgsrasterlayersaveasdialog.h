#ifndef QGSRASTERLAYERSAVEASDIALOG_H
#define QGSRASTERLAYERSAVEASDIALOG_H

#include "ui_qgsrasterlayersaveasdialogbase.h"
#include "qgsrectangle.h"

class QgsRasterDataProvider;

class GUI_EXPORT QgsRasterLayerSaveAsDialog: public QDialog, private Ui::QgsRasterLayerSaveAsDialogBase
{
    Q_OBJECT
  public:
    QgsRasterLayerSaveAsDialog( QgsRasterDataProvider* sourceProvider, const QgsRectangle& currentExtent, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QgsRasterLayerSaveAsDialog();

    int nColumns() const;
    int nRows() const;
    int maximumTileSizeX() const;
    int maximumTileSizeY() const;
    bool tileMode() const;
    QString outputFileName() const;
    QString outputFormat() const;
    QStringList createOptions() const;
    QgsRectangle outputRectangle() const;

    void hideFormat();
    void hideOutput();

  private slots:
    void on_mBrowseButton_clicked();
    void on_mSaveAsLineEdit_textChanged( const QString& text );
    void on_mCurrentExtentButton_clicked();
    void on_mProviderExtentButton_clicked();
    void on_mFormatComboBox_currentIndexChanged( const QString& text );

  private:
    QgsRasterDataProvider* mDataProvider;
    QgsRectangle mCurrentExtent;

    void setValidators();
    void setOutputExtent( const QgsRectangle& r );
};

#endif // QGSRASTERLAYERSAVEASDIALOG_H
