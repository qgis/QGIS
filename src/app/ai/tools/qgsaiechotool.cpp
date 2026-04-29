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
