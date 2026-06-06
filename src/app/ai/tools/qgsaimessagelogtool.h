/***************************************************************************
    qgsaimessagelogtool.h
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

#ifndef QGSAIMESSAGELOGTOOL_H
#define QGSAIMESSAGELOGTOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiMessageLogBuffer;

/**
 * read_message_log: returns recent QGIS Message Log entries for diagnostics.
 */
class APP_EXPORT QgsAiReadMessageLogTool : public QgsAiTool
{
  public:
    explicit QgsAiReadMessageLogTool( QgsAiMessageLogBuffer *buffer );

    QString name() const override { return u"read_message_log"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
    bool isAvailable() const override;

  private:
    static constexpr int MAX_CAPTURE_BYTES = 32768;

    QgsAiMessageLogBuffer *mBuffer = nullptr;
};

#endif // QGSAIMESSAGELOGTOOL_H
