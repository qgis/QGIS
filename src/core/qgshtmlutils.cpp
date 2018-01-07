#include "qgshtmlutils.h"

QString QgsHtmlUtils::buildBulletList( const QStringList &values )
{
  QString s( QStringLiteral( "<ul>" ) );

  for ( const QString &value : values )
  {
    s += QStringLiteral( "<li>%1</li>" ).arg( value );
  }
  s += QLatin1String( "</ul>" );

  return s;
}
