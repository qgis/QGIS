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

#endif // QGSAITOOLSCHEMAUTIL_H
