#ifndef QGSAITOOLSCHEMAUTIL_H
#define QGSAITOOLSCHEMAUTIL_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

inline QJsonObject schemaObject( const QJsonObject &properties, const QJsonArray &required = QJsonArray() )
{
  QJsonObject schema;
  schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
  schema.insert( QStringLiteral( "properties" ), properties );
  if ( !required.isEmpty() )
    schema.insert( QStringLiteral( "required" ), required );
  return schema;
}

inline QJsonObject prop( const QString &type, const QString &description )
{
  QJsonObject p;
  p.insert( QStringLiteral( "type" ), type );
  p.insert( QStringLiteral( "description" ), description );
  return p;
}

#endif // QGSAITOOLSCHEMAUTIL_H
