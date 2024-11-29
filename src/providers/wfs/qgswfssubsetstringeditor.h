/***************************************************************************
    qgswfssubsetstringeditor.h
     --------------------------------------
    Date                 : 15-Nov-2020
    Copyright            : (C) 2020 by Even Rouault
    Email                : even.rouault at spatials.com
****************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWFSSUBSETSTRINGEDITOR_H
#define QGSWFSSUBSETSTRINGEDITOR_H

#include "qgssubsetstringeditorinterface.h"
#include "qgsvectorlayer.h"
#include "qgsguiutils.h"
#include "qgssqlcomposerdialog.h"
#include "qgswfsprovider.h"
#include "qgswfscapabilities.h"

#include <QWidget>

class QgsWfsSubsetStringEditor
{
  public:
    //! Instantiate a QgsSQLComposerDialog
    static QgsSubsetStringEditorInterface *create( QgsVectorLayer *layer, QgsWFSProvider *provider, QWidget *parent, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );
};

class QgsWFSValidatorCallback : public QObject, public QgsSQLComposerDialog::SQLValidatorCallback
{
    Q_OBJECT

  public:
    QgsWFSValidatorCallback( QObject *parent, const QgsWFSDataSourceURI &uri, const QString &allSql, const QgsWfsCapabilities::Capabilities &caps );
    bool isValid( const QString &sql, QString &errorReason, QString &warningMsg ) override;

  private:
    QgsWFSDataSourceURI mURI;
    QString mAllSql;
    const QgsWfsCapabilities::Capabilities mCaps;
};

class QgsWFSTableSelectedCallback : public QObject, public QgsSQLComposerDialog::TableSelectedCallback
{
    Q_OBJECT

  public:
    QgsWFSTableSelectedCallback( QgsSQLComposerDialog *dialog, const QgsWFSDataSourceURI &uri, const QgsWfsCapabilities::Capabilities &caps );
    void tableSelected( const QString &name ) override;

  private:
    QgsSQLComposerDialog *mDialog = nullptr;
    QgsWFSDataSourceURI mURI;
    const QgsWfsCapabilities::Capabilities mCaps;
};

#endif
