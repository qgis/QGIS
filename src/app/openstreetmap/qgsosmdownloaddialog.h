#ifndef QGSOSMDOWNLOADDIALOG_H
#define QGSOSMDOWNLOADDIALOG_H

#include <QDialog>

#include "ui_qgsosmdownloaddialog.h"

class QgsRectangle;

class QgsOSMDownload;

class QgsOSMDownloadDialog : public QDialog, private Ui::QgsOSMDownloadDialog
{
    Q_OBJECT
  public:
    explicit QgsOSMDownloadDialog( QWidget* parent = 0 );
    ~QgsOSMDownloadDialog();

    void setRect( const QgsRectangle& rect );
    void setRectReadOnly( bool readonly );
    QgsRectangle rect() const;

  private:
    void populateLayers();

  private slots:
    void onExtentCanvas();
    void onExtentLayer();
    void onExtentManual();
    void onCurrentLayerChanged( int index );
    void onBrowseClicked();
    void onOK();
    void onClose();
    void onFinished();
    void onDownloadProgress( qint64, qint64 );

  private:
    QgsOSMDownload* mDownload;
};

#endif // QGSOSMDOWNLOADDIALOG_H
