/***************************************************************************
    qgsaiechotool.h
    ---------------------
    begin                : April 2026
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

#ifndef QGSAIECHOTOOL_H
#define QGSAIECHOTOOL_H

#include "qgis_app.h"
#include "qgsaitool.h"

#include <QString>

using namespace Qt::StringLiterals;

/**
 * Smoke-test tool that returns its input under `echoed`. Used to validate the
 * full tool-use loop (schema in payload, model invocation, execute, tool_result
 * back to the model) without depending on any GIS state. Always present in the
 * registry so the loop can be exercised end-to-end on a fresh install.
 */
class APP_EXPORT QgsAiEchoTool : public QgsAiTool
{
  public:
    QgsAiEchoTool() = default;

    QString name() const override { return u"echo"_s; }

    QString description() const override { return u"Returns the provided text unchanged. Use this only when the user explicitly asks to test the tool loop."_s; }

    QJsonObject schema() const override;
    QgsAiToolResult execute( const QJsonObject &args ) override;
};

#endif // QGSAIECHOTOOL_H
