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
#include "qgshistorywidget.h"

#define SIP_NO_FILE

/**
 * Custom QgsHistoryWidget for use with the database query provider.
 *
 * \ingroup gui
 *
 * \note Not available in Python bindings
 * \since QGIS 3.44
 */
class GUI_EXPORT QgsDatabaseQueryHistoryWidget : public QgsHistoryWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsDatabaseQueryHistoryWidget, with the specified \a parent widget.
     *
     * If \a backends is specified then the widget will be filtered to only matching backends.
     *
     * If no \a registry is specified then the singleton QgsHistoryProviderRegistry from QgsGui::historyProviderRegistry()
     * will be used.
     */
    QgsDatabaseQueryHistoryWidget( Qgis::HistoryProviderBackends backends = Qgis::HistoryProviderBackend::LocalProfile, QgsHistoryProviderRegistry *registry = nullptr, const QgsHistoryWidgetContext &context = QgsHistoryWidgetContext(), QWidget *parent = nullptr );

    /**
     * Causes the widget to emit the sqlTriggered() signal.
     */
    void emitSqlTriggered( const QString &connectionUri, const QString &provider, const QString &sql );

  signals:

    /**
     * Emitted when the user has triggered a previously executed SQL statement in the widget.
     */
    void sqlTriggered( const QString &connectionUri, const QString &provider, const QString &sql );
};


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
};

#endif //QGSDBQUERYHISTORYPROVIDER_H
