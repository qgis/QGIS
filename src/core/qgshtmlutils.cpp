/***************************************************************************
    qgshtmlutils.cpp
    ---------------------
    begin                : December 2017
    copyright            : (C) 2017 by Etienne Trimaille
    email                : etienne.trimaille at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/
#include "qgshtmlutils.h"
#include <QStringList>

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
