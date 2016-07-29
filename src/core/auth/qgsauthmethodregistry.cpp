/***************************************************************************
    qgsauthmethodregistry.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthmethodregistry.h"

#include <QString>
#include <QDir>
#include <QLibrary>

#include "qgis.h"
#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsauthmethodmetadata.h"


// typedefs for auth method plugin functions of interest
typedef QString methodkey_t();
typedef QString description_t();
typedef bool    isauthmethod_t();


QgsAuthMethodRegistry *QgsAuthMethodRegistry::instance( const QString& pluginPath )
{
  static QgsAuthMethodRegistry* sInstance( new QgsAuthMethodRegistry( pluginPath ) );
  return sInstance;
}

QgsAuthMethodRegistry::QgsAuthMethodRegistry( const QString& pluginPath )
{
  // At startup, examine the libs in the qgis/lib dir and store those that
  // are an auth method shared lib
  // check all libs in the current plugin directory and get name and descriptions
#if 0
  char **argv = qApp->argv();
  QString appDir = argv[0];
  int bin = appDir.findRev( "/bin", -1, false );
  QString baseDir = appDir.left( bin );
  QString mLibraryDirectory = baseDir + "/lib";
#endif
  mLibraryDirectory = pluginPath;
  mLibraryDirectory.setSorting( QDir::Name | QDir::IgnoreCase );
  mLibraryDirectory.setFilter( QDir::Files | QDir::NoSymLinks );

#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  mLibraryDirectory.setNameFilters( QStringList( "*authmethod.dll" ) );
#else
  mLibraryDirectory.setNameFilters( QStringList( "*authmethod.so" ) );
#endif

  QgsDebugMsg( QString( "Checking for auth method plugins in: %1" ).arg( mLibraryDirectory.path() ) );

  if ( mLibraryDirectory.count() == 0 )
  {
    QString msg = QObject::tr( "No QGIS auth method plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() );
    msg += QObject::tr( "No authentication methods can be used. Check your QGIS installation" );

    QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
    output->setTitle( QObject::tr( "No Authentication Methods" ) );
    output->setMessage( msg, QgsMessageOutput::MessageText );
    output->showMessage();
    return;
  }

  // auth method file regex pattern, only files matching the pattern are loaded if the variable is defined
  QString filePattern = getenv( "QGIS_AUTHMETHOD_FILE" );
  QRegExp fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  QListIterator<QFileInfo> it( mLibraryDirectory.entryInfoList() );
  while ( it.hasNext() )
  {
    QFileInfo fi( it.next() );

    if ( !fileRegexp.isEmpty() )
    {
      if ( fileRegexp.indexIn( fi.fileName() ) == -1 )
      {
        QgsDebugMsg( "auth method " + fi.fileName() + " skipped because doesn't match pattern " + filePattern );
        continue;
      }
    }

    QLibrary myLib( fi.filePath() );
    if ( !myLib.load() )
    {
      QgsDebugMsg( QString( "Checking %1: ...invalid (lib not loadable): %2" ).arg( myLib.fileName(), myLib.errorString() ) );
      continue;
    }

    // get the description and the key for the auth method plugin
    isauthmethod_t *isAuthMethod = reinterpret_cast< isauthmethod_t * >( cast_to_fptr( myLib.resolve( "isAuthMethod" ) ) );
    if ( !isAuthMethod )
    {
      QgsDebugMsg( QString( "Checking %1: ...invalid (no isAuthMethod method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // check to see if this is an auth method plugin
    if ( !isAuthMethod() )
    {
      QgsDebugMsg( QString( "Checking %1: ...invalid (not an auth method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // looks like an auth method plugin. get the key and description
    description_t *pDesc = reinterpret_cast< description_t * >( cast_to_fptr( myLib.resolve( "description" ) ) );
    if ( !pDesc )
    {
      QgsDebugMsg( QString( "Checking %1: ...invalid (no description method)" ).arg( myLib.fileName() ) );
      continue;
    }

    methodkey_t *pKey = reinterpret_cast< methodkey_t * >( cast_to_fptr( myLib.resolve( "authMethodKey" ) ) );
    if ( !pKey )
    {
      QgsDebugMsg( QString( "Checking %1: ...invalid (no authMethodKey method)" ).arg( myLib.fileName() ) );
      continue;
    }

    // add this auth method to the method map
    mAuthMethods[pKey()] = new QgsAuthMethodMetadata( pKey(), pDesc(), myLib.fileName() );

  }
}

// typedef for the unload auth method function
typedef void cleanupAuthMethod_t();

QgsAuthMethodRegistry::~QgsAuthMethodRegistry()
{
  AuthMethods::const_iterator it = mAuthMethods.begin();

  while ( it != mAuthMethods.end() )
  {
    QgsDebugMsg( QString( "cleanup: %1" ).arg( it->first ) );
    QString lib = it->second->library();
    QLibrary myLib( lib );
    if ( myLib.isLoaded() )
    {
      cleanupAuthMethod_t* cleanupFunc = reinterpret_cast< cleanupAuthMethod_t* >( cast_to_fptr( myLib.resolve( "cleanupAuthMethod" ) ) );
      if ( cleanupFunc )
        cleanupFunc();
    }
    // clear cached QgsAuthMethodMetadata *
    delete it->second;
    ++it;
  }
}


/** Convenience function for finding any existing auth methods that match "authMethodKey"

  Necessary because [] map operator will create a QgsProviderMetadata
  instance.  Also you cannot use the map [] operator in const members for that
  very reason.  So there needs to be a convenient way to find an auth method
  without accidentally adding a null meta data item to the metadata map.
*/
static QgsAuthMethodMetadata * findMetadata_( QgsAuthMethodRegistry::AuthMethods const & metaData,
    QString const & authMethodKey )
{
  QgsAuthMethodRegistry::AuthMethods::const_iterator i =
    metaData.find( authMethodKey );

  if ( i != metaData.end() )
  {
    return i->second;
  }

  return nullptr;
} // findMetadata_


QString QgsAuthMethodRegistry::library( const QString &authMethodKey ) const
{
  QgsAuthMethodMetadata * md = findMetadata_( mAuthMethods, authMethodKey );

  if ( md )
  {
    return md->library();
  }

  return QString();
}

QString QgsAuthMethodRegistry::pluginList( bool asHtml ) const
{
  AuthMethods::const_iterator it = mAuthMethods.begin();

  if ( mAuthMethods.empty() )
  {
    return QObject::tr( "No authentication method plugins are available." );
  }

  QString list;

  if ( asHtml )
  {
    list += "<ol>";
  }

  while ( it != mAuthMethods.end() )
  {
    if ( asHtml )
    {
      list += "<li>";
    }

    list += it->second->description();

    if ( asHtml )
    {
      list + "<br></li>";
    }
    else
    {
      list += '\n';
    }

    ++it;
  }

  if ( asHtml )
  {
    list += "</ol>";
  }

  return list;
}

const QDir &QgsAuthMethodRegistry::libraryDirectory() const
{
  return mLibraryDirectory;
}

void QgsAuthMethodRegistry::setLibraryDirectory( const QDir &path )
{
  mLibraryDirectory = path;
}


// typedef for the QgsDataProvider class factory
typedef QgsAuthMethod * classFactoryFunction_t();

QgsAuthMethod *QgsAuthMethodRegistry::authMethod( const QString &authMethodKey )
{
  // load the plugin
  QString lib = library( authMethodKey );

#ifdef TESTAUTHMETHODLIB
  const char *cLib = lib.toUtf8();

  // test code to help debug auth method plugin loading problems
  //  void *handle = dlopen(cLib, RTLD_LAZY);
  void *handle = dlopen( cOgrLib, RTLD_LAZY | RTLD_GLOBAL );
  if ( !handle )
  {
    QgsLogger::warning( "Error in dlopen" );
  }
  else
  {
    QgsDebugMsg( "dlopen succeeded" );
    dlclose( handle );
  }

#endif
  // load the auth method
  QLibrary myLib( lib );

  QgsDebugMsg( "Auth method library name is " + myLib.fileName() );
  if ( !myLib.load() )
  {
    QgsMessageLog::logMessage( QObject::tr( "Failed to load %1: %2" ).arg( lib, myLib.errorString() ) );
    return nullptr;
  }

  classFactoryFunction_t *classFactory = reinterpret_cast< classFactoryFunction_t * >( cast_to_fptr( myLib.resolve( "classFactory" ) ) );
  if ( !classFactory )
  {
    QgsDebugMsg( QString( "Failed to load %1: no classFactory method" ).arg( lib ) );
    return nullptr;
  }

  QgsAuthMethod *authMethod = classFactory();
  if ( !authMethod )
  {
    QgsMessageLog::logMessage( QObject::tr( "Unable to instantiate the auth method plugin %1" ).arg( lib ) );
    myLib.unload();
    return nullptr;
  }

  QgsDebugMsg( QString( "Instantiated the auth method plugin: %1" ).arg( authMethod->key() ) );
  return authMethod;
}

typedef QWidget * editFactoryFunction_t( QWidget * parent );

QWidget *QgsAuthMethodRegistry::editWidget( const QString &authMethodKey, QWidget *parent )
{
  editFactoryFunction_t * editFactory =
    reinterpret_cast< editFactoryFunction_t * >( cast_to_fptr( function( authMethodKey, "editWidget" ) ) );

  if ( !editFactory )
    return nullptr;

  return editFactory( parent );
}

#if QT_VERSION >= 0x050000
QFunctionPointer QgsAuthMethodRegistry::function( QString const & authMethodKey,
    QString const & functionName )
{
  QLibrary myLib( library( authMethodKey ) );

  QgsDebugMsg( "Library name is " + myLib.fileName() );

  if ( myLib.load() )
  {
    return myLib.resolve( functionName.toAscii().data() );
  }
  else
  {
    QgsDebugMsg( "Cannot load library: " + myLib.errorString() );
    return 0;
  }
}
#else
void *QgsAuthMethodRegistry::function( QString const & authMethodKey,
                                       QString const & functionName )
{
  QLibrary myLib( library( authMethodKey ) );

  QgsDebugMsg( "Library name is " + myLib.fileName() );

  if ( myLib.load() )
  {
    return myLib.resolve( functionName.toAscii().data() );
  }
  else
  {
    QgsDebugMsg( "Cannot load library: " + myLib.errorString() );
    return nullptr;
  }
}
#endif

QLibrary *QgsAuthMethodRegistry::authMethodLibrary( const QString &authMethodKey ) const
{
  QLibrary *myLib = new QLibrary( library( authMethodKey ) );

  QgsDebugMsg( "Library name is " + myLib->fileName() );

  if ( myLib->load() )
    return myLib;

  QgsDebugMsg( "Cannot load library: " + myLib->errorString() );

  delete myLib;

  return nullptr;
}

QStringList QgsAuthMethodRegistry::authMethodList() const
{
  QStringList lst;
  for ( AuthMethods::const_iterator it = mAuthMethods.begin(); it != mAuthMethods.end(); ++it )
  {
    lst.append( it->first );
  }
  return lst;
}

const QgsAuthMethodMetadata *QgsAuthMethodRegistry::authMethodMetadata( const QString &authMethodKey ) const
{
  return findMetadata_( mAuthMethods, authMethodKey );
}
