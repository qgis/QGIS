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

/** \ingroup gui
 * \class QgsManageConnectionsDialog
 */
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
      MSSQL,
      DB2,
      WCS,
      Oracle,
    };

    // constructor
    // mode argument must be 0 for export and 1 for import
    // type argument must be 0 for WMS and 1 for PostGIS
    QgsManageConnectionsDialog( QWidget *parent = nullptr, Mode mode = Export, Type type = WMS, const QString& fileName = "" );

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();
    void selectionChanged();

  private:
    bool populateConnections();

    QDomDocument saveOWSConnections( const QStringList &connections, const QString &service );
    QDomDocument saveWFSConnections( const QStringList &connections );
    QDomDocument savePgConnections( const QStringList & connections );
    QDomDocument saveMssqlConnections( const QStringList & connections );
    QDomDocument saveOracleConnections( const QStringList & connections );
    QDomDocument saveDb2Connections( const QStringList & connections );

    void loadOWSConnections( const QDomDocument &doc, const QStringList &items, const QString &service );
    void loadWFSConnections( const QDomDocument &doc, const QStringList &items );
    void loadPgConnections( const QDomDocument &doc, const QStringList &items );
    void loadMssqlConnections( const QDomDocument &doc, const QStringList &items );
    void loadOracleConnections( const QDomDocument &doc, const QStringList &items );
    void loadDb2Connections( const QDomDocument &doc, const QStringList &items );

    QString mFileName;
    Mode mDialogMode;
    Type mConnectionType;
};

#endif // QGSMANAGECONNECTIONSDIALOG_H

