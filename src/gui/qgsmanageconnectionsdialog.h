/***************************************************************************
    qgsmanageconnectionsdialog.h
    ---------------------
    begin                : Dec 2009
    copyright            : (C) 2009 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMANAGECONNECTIONSDIALOG_H
#define QGSMANAGECONNECTIONSDIALOG_H

#include <QDialog>
#include <QDomDocument>
#include "ui_qgsmanageconnectionsdialogbase.h"

class GUI_EXPORT QgsManageConnectionsDialog : public QDialog, private Ui::QgsManageConnectionsDialogBase
{
    Q_OBJECT

  public:
    enum Mode
    {
      Export,
      Import
    };

    enum Type
    {
      WMS,
      PostGIS,
      WFS,
    };

    // constructor
    // mode argument must be 0 for export and 1 for import
    // type argument must be 0 for WMS and 1 for PostGIS
    QgsManageConnectionsDialog( QWidget *parent = NULL, Mode mode = Export, Type type = WMS, QString fileName = "" );

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();

  private:
    bool populateConnections();
    QDomDocument saveWMSConnections( const QStringList &connections );
    QDomDocument saveWFSConnections( const QStringList &connections );
    QDomDocument savePgConnections( const QStringList & connections );
    void loadWMSConnections( const QDomDocument &doc, const QStringList &items );
    void loadWFSConnections( const QDomDocument &doc, const QStringList &items );
    void loadPgConnections( const QDomDocument &doc, const QStringList &items );

    QString mFileName;
    Mode mDialogMode;
    Type mConnectionType;
};

#endif // QGSMANAGECONNECTIONSDIALOG_H

