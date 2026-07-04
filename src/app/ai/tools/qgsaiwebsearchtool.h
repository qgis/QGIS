/***************************************************************************
    qgsaiwebsearchtool.h
    --------------------
    begin                : July 2026
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

#ifndef QGSAIWEBSEARCHTOOL_H
#define QGSAIWEBSEARCHTOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

class QgsAiModelRouter;

class APP_EXPORT QgsAiWebSearchToolBase : public QgsAiTool
{
  public:
    explicit QgsAiWebSearchToolBase( QgsAiModelRouter *router );

    bool isAvailable() const override;
    QString availabilityReason() const override;

  protected:
    QgsAiToolResult postSearch( const QString &path, const QJsonObject &body ) const;

    QgsAiModelRouter *mRouter = nullptr;
};

class APP_EXPORT QgsAiWebSearchTool : public QgsAiWebSearchToolBase
{
  public:
    using QgsAiWebSearchToolBase::QgsAiWebSearchToolBase;

    QString name() const override { return u"web_search"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

class APP_EXPORT QgsAiCatalogSearchTool : public QgsAiWebSearchToolBase
{
  public:
    using QgsAiWebSearchToolBase::QgsAiWebSearchToolBase;

    QString name() const override { return u"catalog_search"_s; }
    QString description() const override;
    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

#endif // QGSAIWEBSEARCHTOOL_H
