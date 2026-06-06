/***************************************************************************
    qgsaimessagelogbuffer.h
    ---------------------
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

#ifndef QGSAIMESSAGELOGBUFFER_H
#define QGSAIMESSAGELOGBUFFER_H

#include "qgis_app.h"
#include "qgis.h"

#include <QDateTime>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

/**
 * In-memory ring buffer of recent QgsMessageLog entries for AI diagnostics.
 *
 * Connects to QgsApplication::messageLog() and retains the most recent entries
 * so the read_message_log tool can query them on demand.
 */
class APP_EXPORT QgsAiMessageLogBuffer : public QObject
{
    Q_OBJECT

  public:
    struct Entry
    {
        QDateTime timestamp;
        QString tag;
        Qgis::MessageLevel level = Qgis::MessageLevel::Info;
        QString message;
    };

    struct Query
    {
      QList<Qgis::MessageLevel> levels;
      QStringList tags;
      QString search;
      int limit = 50;
      int sinceSeconds = -1;
    };

    struct QueryResult
    {
      QList<Entry> entries;
      int totalBuffered = 0;
      bool truncated = false;
    };

    static constexpr int DEFAULT_CAPACITY = 2000;
    static constexpr int MAX_QUERY_LIMIT = 200;

    explicit QgsAiMessageLogBuffer( QObject *parent = nullptr, int capacity = DEFAULT_CAPACITY );

    QueryResult query( const Query &query ) const;
    QStringList distinctTags() const;
    int entryCount() const;

  private slots:
    void onMessageReceived( const QString &message, const QString &tag, Qgis::MessageLevel level, Qgis::StringFormat format );

  private:
    mutable QMutex mMutex;
    QVector<Entry> mEntries;
    int mCapacity;
};

#endif // QGSAIMESSAGELOGBUFFER_H
