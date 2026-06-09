/***************************************************************************
    qgsaiindexingscheduler.h
    ------------------------
    begin                : June 2026
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

#ifndef QGSAIINDEXINGSCHEDULER_H
#define QGSAIINDEXINGSCHEDULER_H

#include "qgis_app.h"

#include <QObject>
#include <QPointer>
#include <QTimer>

class QgsAiWorkspaceIndex;
class QgsTask;

class APP_EXPORT QgsAiIndexingScheduler : public QObject
{
  public:
    explicit QgsAiIndexingScheduler( QgsAiWorkspaceIndex *index, QObject *parent = nullptr );

    bool automaticEnabled() const { return mAutomaticEnabled; }
    void setAutomaticEnabled( bool enabled );
    void scheduleStartupIndexing( int delayMs = 10000 );
    void scheduleWorkspaceIndexing( int delayMs = 5000 );
    void cancel();

  private:
    void startWorkspaceIndexing();
    QPointer<QgsAiWorkspaceIndex> mIndex;
    QPointer<QgsTask> mRunningTask;
    QTimer mDebounceTimer;
    bool mAutomaticEnabled = true;
};

#endif // QGSAIINDEXINGSCHEDULER_H
