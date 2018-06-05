/***************************************************************************
  qgsquickmessagelogmodel.h
  --------------------------------------
  date                 : 13.7.2016
  copyright            : (C) 2016 by Matthias Kuhn
  email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKMESSAGELOGMODEL_H
#define QGSQUICKMESSAGELOGMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QVector>

#include "qgis.h"
#include "qgsmessagelog.h"

#include "qgis_quick.h"

/**
 * \ingroup quick
 *
 * This model will connect to the QgsMessageLog singleton and store any
 * messages received. Can be used as a model for QListView,
 * for example QgsQuick.MessageLog (QML only)
 *
 * \note QML Type: MessageLogModel
 *
 * \sa QgsMessageLog
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickMessageLogModel : public QAbstractListModel
{
    Q_OBJECT

    struct LogMessage
    {
      LogMessage() = default;

      QString tag;
      QString message;
      Qgis::MessageLevel level;
    };

    enum Roles
    {
      MessageRole = Qt::UserRole,
      MessageTagRole,
      MessageLevelRole
    };

  public:
    //! Create new message log model
    QgsQuickMessageLogModel( QObject *parent = nullptr );

    QHash<int, QByteArray> roleNames() const override;
    int rowCount( const QModelIndex &parent ) const override;
    QVariant data( const QModelIndex &index, int role ) const override;

  private slots:
    void onMessageReceived( const QString &message, const QString &tag, Qgis::MessageLevel level );

  private:
    //! LogMessage factory
    LogMessage logMessage( const QString &tag, const QString &message, Qgis::MessageLevel level );

    //! Storage of all messages.
    QVector<LogMessage> mMessages;
};

#endif // QGSQUICKMESSAGELOGMODEL_H
