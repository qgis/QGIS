#include "qgsfileutils.h"
#include <QObject>

QString QgsFileUtils::representFileSize( qint64 bytes )
{
  QStringList list;
  list << QObject::tr( "KB" ) << QObject::tr( "MB" ) << QObject::tr( "GB" ) << QObject::tr( "TB" );

  QStringListIterator i( list );
  QString unit = QObject::tr( "bytes" );

  while ( bytes >= 1024.0 && i.hasNext() )
  {
    unit = i.next();
    bytes /= 1024.0;
  }
  return QString( "%1 %2" ).arg( QString::number( bytes ), unit );
}
