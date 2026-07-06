/***************************************************************************
    qgsaiwebfetchtool.h
    -------------------
    begin                : July 2026
    copyright            : (C) 2026 by Valerio Sota
    email                : valeriosota.dev at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSAIWEBFETCHTOOL_H
#define QGSAIWEBFETCHTOOL_H

#include "qgis_app.h"
#include "qgsaiwebsearchtool.h"

#include <QString>

using namespace Qt::StringLiterals;

class APP_EXPORT QgsAiWebFetchTool : public QgsAiWebSearchToolBase
{
  public:
    using QgsAiWebSearchToolBase::QgsAiWebSearchToolBase;

    QString name() const override { return u"web_fetch"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

#endif // QGSAIWEBFETCHTOOL_H
