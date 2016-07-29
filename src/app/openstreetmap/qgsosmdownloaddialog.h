/***************************************************************************
  qgsosmdownloaddialog.h
  --------------------------------------
  Date                 : February 2013
  Copyright            : (C) 2013 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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
    explicit QgsOSMDownloadDialog( QWidget* parent = nullptr );
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
