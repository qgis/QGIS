/***************************************************************************
    qgsstacdownloadassetsdialog.h
    ---------------------
    begin                : September 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACDOWNLOADASSETSDIALOG_H
#define QGSSTACDOWNLOADASSETSDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsstacitem.h"
#include "ui_qgsstacdownloadassetsdialog.h"

#include <QDialog>

class QgsMessageBar;

class QgsStacDownloadAssetsDialog : public QDialog, private Ui::QgsStacDownloadAssetsDialog
{
    Q_OBJECT

  public:
    explicit QgsStacDownloadAssetsDialog( QWidget *parent = nullptr );

    void accept() override;

    void setAuthCfg( const QString &authCfg );
    void setMessageBar( QgsMessageBar *bar );
    void setStacItem( QgsStacItem *stacItem );
    QString selectedFolder();
    QStringList selectedUrls();

  private slots:
    void showContextMenu( QPoint p );

  private:
    void selectAll();
    void deselectAll();

    QMenu *mContextMenu = nullptr;
    QString mAuthCfg;
    QgsMessageBar *mMessageBar = nullptr;
};

///@endcond

#endif // QGSSTACDOWNLOADASSETSDIALOG_H
