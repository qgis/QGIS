/***************************************************************************
    qgsailayerindexcoordinator.h
    ---------------------
    begin                : May 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAILAYERINDEXCOORDINATOR_H
#define QGSAILAYERINDEXCOORDINATOR_H

#include "qgis_app.h"

#include <QObject>
#include <QPointer>
#include <QSet>
#include <QString>
#include <QTimer>

class QgsAiWorkspaceIndex;
class QgsMapLayer;
class QgsProject;
class QgsTask;

/**
 * Listens to QgsProject layer-add/remove events and to per-layer data/editing
 * signals, batching the affected layer ids and re-indexing them via QgsAiWorkspaceIndex
 * after a debounce window. Removed layers are deleted from the index immediately.
 *
 * Disabled by default: setEnabled(true) wires the QgsProject signals; setEnabled(false)
 * disconnects them. This lets the user opt in / out without recreating the coordinator.
 */
class APP_EXPORT QgsAiLayerIndexCoordinator : public QObject
{
    Q_OBJECT

  public:
    explicit QgsAiLayerIndexCoordinator( QgsAiWorkspaceIndex *index, QObject *parent = nullptr );

    bool isEnabled() const { return mEnabled; }
    void setEnabled( bool enabled );

    int debounceMs() const { return mDebounceMs; }
    void setDebounceMs( int ms );
    int bulkDebounceMs() const { return mBulkDebounceMs; }
    void setBulkDebounceMs( int ms );
    void scheduleAllLayers();

    /**
     * Override the QgsProject the coordinator listens to. Defaults to QgsProject::instance()
     * when null is passed. Useful in tests.
     */
    void setProject( QgsProject *project );

  signals:
    void reindexStarted( const QString &layerId );
    void reindexFinished( const QString &layerId, bool success, const QString &errorMessage );

  private slots:
    void onLayerAdded( QgsMapLayer *layer );
    void onLayerWillBeRemoved( const QString &layerId );
    void onLayerChanged();
    void onVectorLayerCommitted( const QString &layerId );
    void flushDirty();

  private:
    void connectProjectSignals();
    void disconnectProjectSignals();
    void connectLayerSignals( QgsMapLayer *layer );
    void scheduleDirty( const QString &layerId );
    void startDebounceTimer();

    QgsAiWorkspaceIndex *mIndex = nullptr;
    QgsProject *mProject = nullptr;
    QSet<QString> mDirtyLayers;
    QTimer mDebounceTimer;
    QPointer<QgsTask> mRunningTask;
    bool mEnabled = false;
    bool mUseBulkDebounce = false;
    int mDebounceMs = 5000;
    int mBulkDebounceMs = 15000;
};

#endif // QGSAILAYERINDEXCOORDINATOR_H
