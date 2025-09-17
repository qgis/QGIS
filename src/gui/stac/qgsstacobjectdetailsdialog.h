/***************************************************************************
    qgsstacobjectdetailsdialog.h
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

#ifndef QGSSTACOBJECTDETAILSDIALOG_H
#define QGSSTACOBJECTDETAILSDIALOG_H

///@cond PRIVATE
#define SIP_NO_FILE

#include "qgsstacasset.h"
#include "qgsstacobject.h"
#include "ui_qgsstacobjectdetailsdialog.h"

#include <QDialog>

class QgsStacObjectDetailsDialog : public QDialog, private Ui::QgsStacObjectDetailsDialog
{
    Q_OBJECT

  public:
    explicit QgsStacObjectDetailsDialog( QWidget *parent = nullptr );

    void setAuthcfg( const QString &authcfg );

    void setContentFromStacObject( QgsStacObject *stacObject );
    void setContentFromStacAsset( const QString &assetId, const QgsStacAsset *stacAsset );

  private:
    QString mAuthcfg;
    void setContent( QString bodyHtml, QString thumbnailHtml );
    bool isThumbnailAsset( const QgsStacAsset *stacAsset );
    QString thumbnailHtmlContent( const QgsStacAsset *stacAsset );
};

///@endcond

#endif // QGSSTACOBJECTDETAILSDIALOG_H
