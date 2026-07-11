/***************************************************************************
    qgsaitoolschemautil.h
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

#ifndef QGSAITOOLSCHEMAUTIL_H
#define QGSAITOOLSCHEMAUTIL_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

using namespace Qt::StringLiterals;

inline QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
{
  QJsonObject schema;
  schema.insert( u"type"_s, u"object"_s );
  schema.insert( u"properties"_s, properties );
  if ( !required.isEmpty() )
    schema.insert( u"required"_s, required );
  return schema;
}

inline QJsonObject prop( const QString &type, const QString &description )
{
  QJsonObject p;
  p.insert( u"type"_s, type );
  p.insert( u"description"_s, description );
  return p;
}

inline QJsonObject propArray( const QJsonObject &items, const QString &description )
{
  QJsonObject p;
  p.insert( u"type"_s, u"array"_s );
  p.insert( u"items"_s, items );
  p.insert( u"description"_s, description );
  return p;
}

#endif // QGSAITOOLSCHEMAUTIL_H
