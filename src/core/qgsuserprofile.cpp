#include "qgsuserprofile.h"
#include "qgsapplication.h"

#include <QDir>
#include <QTextStream>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>

QgsUserProfile::QgsUserProfile( QString folder )
{
  mProfileFolder = folder;
}

const QString QgsUserProfile::folder() const
{
  return mProfileFolder;
}

QgsError QgsUserProfile::validate() const
{
  QgsError error;
  if ( !QDir( mProfileFolder ).exists() )
  {
    error.append( QObject::tr( "Profile folder doesn't exist" ) );
  }
  return error;
}

const QString QgsUserProfile::name() const
{
  QDir dir( mProfileFolder );
  return dir.dirName();
}

void QgsUserProfile::initSettings() const
{
  // tell QSettings to use INI format and save the file in custom config path
  QSettings::setDefaultFormat( QSettings::IniFormat );
  QSettings::setPath( QSettings::IniFormat, QSettings::UserScope, folder() );
}

const QString QgsUserProfile::alias() const
{
  QFile qgisPrivateDbFile( qgisDB() );

  // Looks for qgis.db
  // If it's not there we can just return name.
  if ( !qgisPrivateDbFile.exists() )
  {
    return name();
  }

  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
  db.setDatabaseName( qgisDB() );
  if ( !db.open() )
    return name();

  QSqlQuery query;
  query.prepare( "SELECT value FROM tbl_config_variables WHERE variable = 'ALIAS'" );
  QString profileAlias = name();
  if ( query.exec() )
  {
    query.next();
    QString alias = query.value( 0 ).toString();
    if ( !alias.isEmpty() )
      profileAlias = alias;
  }
  db.close();
  return profileAlias;
}

bool QgsUserProfile::setAlias( const QString &alias )
{
  QFile qgisPrivateDbFile( qgisDB() );

  if ( !qgisPrivateDbFile.exists() )
  {
    return false;
  }

  QSqlDatabase db = QSqlDatabase::addDatabase( "QSQLITE" );
  db.setDatabaseName( qgisDB() );
  if ( !db.open() )
    return false;

  QSqlQuery query;
  QString sql = "INSERT OR REPLACE INTO tbl_config_variables VALUES ('ALIAS', :alias);";
  query.prepare( sql );
  query.bindValue( ":alias", alias );
  query.exec();
  db.close();
  return true;
}

const QIcon QgsUserProfile::icon() const
{
  QString path = mProfileFolder + QDir::separator() + "icon.svg";
  if ( !QDir( path ).exists() )
  {
    return QgsApplication::getThemeIcon( "user.svg" );
  }
  return QIcon( path );
}

QString QgsUserProfile::qgisDB() const
{
  return mProfileFolder + QDir::separator() + "qgis.db" ;
}
