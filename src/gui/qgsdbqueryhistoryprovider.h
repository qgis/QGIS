/***************************************************************************
                            qgsdbqueryhistoryprovider.h
                            --------------------------
    begin                : April 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDBQUERYHISTORYPROVIDER_H
#define QGSDBQUERYHISTORYPROVIDER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include "qgshistoryprovider.h"
#include "qgshistoryentrynode.h"

#define SIP_NO_FILE

/**
 * History provider for operations database queries.
 *
 * \ingroup gui
 *
 * \note Not available in Python bindings
 * \since QGIS 3.32
 */
class GUI_EXPORT QgsDatabaseQueryHistoryProvider : public QgsAbstractHistoryProvider
{
    Q_OBJECT

  public:
    QgsDatabaseQueryHistoryProvider();

    QString id() const override;

    QgsHistoryEntryNode *createNodeForEntry( const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context ) override SIP_FACTORY;
    void updateNodeForEntry( QgsHistoryEntryNode *node, const QgsHistoryEntry &entry, const QgsHistoryWidgetContext &context ) override;

    /**
     * Causes the provider to emit the openSqlDialog() signal.
     */
    void emitOpenSqlDialog( const QString &connectionUri, const QString &provider, const QString &sql );

  signals:

    /**
     * Emitted when the provider wants to trigger a SQL execution dialog.
     */
    void openSqlDialog( const QString &connectionUri, const QString &provider, const QString &sql );
};

#endif //QGSDBQUERYHISTORYPROVIDER_H
