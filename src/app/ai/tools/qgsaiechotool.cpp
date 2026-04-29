#include "qgsaiechotool.h"

#include <QJsonArray>

QJsonObject QgsAiEchoTool::schema() const
{
  QJsonObject properties;
  QJsonObject textProp;
  textProp.insert( QStringLiteral( "type" ), QStringLiteral( "string" ) );
  textProp.insert( QStringLiteral( "description" ), QStringLiteral( "Text to echo back unchanged." ) );
  properties.insert( QStringLiteral( "text" ), textProp );

  QJsonObject schema;
  schema.insert( QStringLiteral( "type" ), QStringLiteral( "object" ) );
  schema.insert( QStringLiteral( "properties" ), properties );
  schema.insert( QStringLiteral( "required" ), QJsonArray { QStringLiteral( "text" ) } );
  return schema;
}

QgsAiToolResult QgsAiEchoTool::execute( const QJsonObject &args )
{
  const QString text = args.value( QStringLiteral( "text" ) ).toString();
  QJsonObject output;
  output.insert( QStringLiteral( "echoed" ), text );
  return QgsAiToolResult::ok( output );
}
