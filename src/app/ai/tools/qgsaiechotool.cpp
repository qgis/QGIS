/***************************************************************************
    qgsaiechotool.cpp
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

#include "qgsaiechotool.h"

#include <QJsonArray>
#include <QString>

using namespace Qt::StringLiterals;

QJsonObject QgsAiEchoTool::schema() const
{
  QJsonObject properties;
  QJsonObject textProp;
  textProp.insert( u"type"_s, u"string"_s );
  textProp.insert( u"description"_s, u"Text to echo back unchanged."_s );
  properties.insert( u"text"_s, textProp );

  QJsonObject schema;
  schema.insert( u"type"_s, u"object"_s );
  schema.insert( u"properties"_s, properties );
  schema.insert( u"required"_s, QJsonArray { u"text"_s } );
  return schema;
}

QgsAiToolResult QgsAiEchoTool::execute( const QJsonObject &args )
{
  const QString text = args.value( u"text"_s ).toString();
  QJsonObject output;
  output.insert( u"echoed"_s, text );
  return QgsAiToolResult::ok( output );
}
