#include "qgsaitoolregistry.h"

#include <QJsonObject>
#include <QSet>

QgsAiToolRegistry::QgsAiToolRegistry( QObject *parent )
  : QObject( parent )
{
}

QgsAiToolRegistry::~QgsAiToolRegistry() = default;

bool QgsAiToolRegistry::registerTool( std::unique_ptr<QgsAiTool> tool )
{
  if ( !tool )
    return false;

  const QString toolName = tool->name();
  if ( toolName.isEmpty() || mTools.contains( toolName ) )
    return false;

  mTools.emplace( toolName, std::move( tool ) );
  emit toolRegistered( toolName );
  return true;
}

QgsAiTool *QgsAiToolRegistry::find( const QString &name ) const
{
  const auto it = mTools.find( name );
  return it == mTools.end() ? nullptr : it->second.get();
}

QStringList QgsAiToolRegistry::toolNames() const
{
  QStringList names;
  names.reserve( static_cast<int>( mTools.size() ) );
  for ( const auto &entry : mTools )
    names.append( entry.first );
  return names;
}

QJsonArray QgsAiToolRegistry::schemasJson( const QStringList &allowedTools ) const
{
  const QSet<QString> filter( allowedTools.begin(), allowedTools.end() );
  QJsonArray array;
  for ( auto it = mTools.cbegin(); it != mTools.cend(); ++it )
  {
    if ( !filter.isEmpty() && !filter.contains( it->first ) )
      continue;

    const QgsAiTool *tool = it->second.get();
    QJsonObject entry;
    entry.insert( QStringLiteral( "name" ), tool->name() );
    entry.insert( QStringLiteral( "description" ), tool->description() );
    entry.insert( QStringLiteral( "input_schema" ), tool->schema() );
    array.append( entry );
  }
  return array;
}

void QgsAiToolRegistry::clear()
{
  mTools.clear();
}
