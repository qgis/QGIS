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
#include "qgis_gui.h"
#include "qgis_sip.h"

/**
 * \ingroup gui
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
      WCS,
      Oracle,
      HANA,
      GeoNode,
      XyzTiles,
      ArcgisMapServer,
      ArcgisFeatureServer,
      VectorTile
    };

    /**
     * Constructor for QgsManageConnectionsDialog.
     */
    QgsManageConnectionsDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Mode mode = Export, Type type = WMS, const QString &fileName = QString() );

  public slots:
    void doExportImport();
    void selectAll();
    void clearSelection();
    void selectionChanged();

  private:
    bool populateConnections();

    QDomDocument saveOWSConnections( const QStringList &connections, const QString &service );
    QDomDocument saveWfsConnections( const QStringList &connections );
    QDomDocument savePgConnections( const QStringList &connections );
    QDomDocument saveMssqlConnections( const QStringList &connections );
    QDomDocument saveOracleConnections( const QStringList &connections );
    QDomDocument saveHanaConnections( const QStringList &connections );
    QDomDocument saveGeonodeConnections( const QStringList &connections );
    QDomDocument saveXyzTilesConnections( const QStringList &connections );
    QDomDocument saveArcgisConnections( const QStringList &connections, const QString &service );
    QDomDocument saveVectorTileConnections( const QStringList &connections );

    void loadOWSConnections( const QDomDocument &doc, const QStringList &items, const QString &service );
    void loadWfsConnections( const QDomDocument &doc, const QStringList &items );
    void loadPgConnections( const QDomDocument &doc, const QStringList &items );
    void loadMssqlConnections( const QDomDocument &doc, const QStringList &items );
    void loadOracleConnections( const QDomDocument &doc, const QStringList &items );
    void loadHanaConnections( const QDomDocument &doc, const QStringList &items );
    void loadGeonodeConnections( const QDomDocument &doc, const QStringList &items );
    void loadXyzTilesConnections( const QDomDocument &doc, const QStringList &items );
    void loadArcgisConnections( const QDomDocument &doc, const QStringList &items, const QString &service );
    void loadVectorTileConnections( const QDomDocument &doc, const QStringList &items );

    QString mFileName;
    Mode mDialogMode;
    Type mConnectionType;
};

// clazy:excludeall=qstring-allocations

#endif // QGSMANAGECONNECTIONSDIALOG_H

