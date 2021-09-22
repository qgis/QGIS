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

#include "qgis.h"
#include "qgsauthconfig.h"
#include "qgsauthmethod.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgsmessagelog.h"
#include "qgsauthmethodmetadata.h"

#ifdef HAVE_STATIC_PROVIDERS
#include "qgsauthbasicmethod.h"
#include "qgsauthesritokenmethod.h"
#include "qgsauthidentcertmethod.h"
#ifdef HAVE_OAUTH2_PLUGIN
#include "qgsauthoauth2method.h"
#endif
#include "qgsauthpkipathsmethod.h"
#include "qgsauthpkcs12method.h"
#endif

#include <QString>
#include <QDir>
#include <QLibrary>
#include <QRegularExpression>


static QgsAuthMethodRegistry *sInstance = nullptr;


QgsAuthMethodRegistry *QgsAuthMethodRegistry::instance( const QString &pluginPath )
{
  if ( !sInstance )
  {
    static QMutex sMutex;
    const QMutexLocker locker( &sMutex );
    if ( !sInstance )
    {
      sInstance = new QgsAuthMethodRegistry( pluginPath );
    }
  }
  return sInstance;
}

/**
 * Convenience function for finding any existing auth methods that match "authMethodKey"
 *
 * Necessary because [] map operator will create a QgsProviderMetadata
 * instance.  Also you cannot use the map [] operator in const members for that
 * very reason.  So there needs to be a convenient way to find an auth method
 * without accidentally adding a null meta data item to the metadata map.
*/
static QgsAuthMethodMetadata *findMetadata_( QgsAuthMethodRegistry::AuthMethods const &metaData,
    QString const &authMethodKey )
{
  const QgsAuthMethodRegistry::AuthMethods::const_iterator i =
    metaData.find( authMethodKey );

  if ( i != metaData.end() )
  {
    return i->second;
  }

  return nullptr;
}

QgsAuthMethodRegistry::QgsAuthMethodRegistry( const QString &pluginPath )
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
  mLibraryDirectory.setPath( pluginPath );
  mLibraryDirectory.setSorting( QDir::Name | QDir::IgnoreCase );
  mLibraryDirectory.setFilter( QDir::Files | QDir::NoSymLinks );

  init();
}

void QgsAuthMethodRegistry::init()
{
#ifdef HAVE_STATIC_PROVIDERS
  mAuthMethods[ QgsAuthBasicMethod::AUTH_METHOD_KEY] = new QgsAuthBasicMethodMetadata();
  mAuthMethods[ QgsAuthEsriTokenMethod::AUTH_METHOD_KEY] = new QgsAuthEsriTokenMethodMetadata();
  mAuthMethods[ QgsAuthIdentCertMethod::AUTH_METHOD_KEY] = new QgsAuthIdentCertMethodMetadata();
#ifdef HAVE_OAUTH2_PLUGIN
  mAuthMethods[ QgsAuthOAuth2Method::AUTH_METHOD_KEY] = new QgsAuthOAuth2MethodMetadata();
#endif
  mAuthMethods[ QgsAuthPkiPathsMethod::AUTH_METHOD_KEY] = new QgsAuthPkiPathsMethodMetadata();
  mAuthMethods[ QgsAuthPkcs12Method::AUTH_METHOD_KEY] = new QgsAuthPkcs12MethodMetadata();
#else
  typedef QgsAuthMethodMetadata *factory_function( );

#if defined(Q_OS_WIN) || defined(__CYGWIN__)
  mLibraryDirectory.setNameFilters( QStringList( "*authmethod_*.dll" ) );
#else
  mLibraryDirectory.setNameFilters( QStringList( QStringLiteral( "*authmethod_*.so" ) ) );
#endif
  QgsDebugMsgLevel( QStringLiteral( "Checking for auth method plugins in: %1" ).arg( mLibraryDirectory.path() ), 2 );

  if ( mLibraryDirectory.count() == 0 )
  {
    QString msg = QObject::tr( "No QGIS auth method plugins found in:\n%1\n" ).arg( mLibraryDirectory.path() );
    msg += QObject::tr( "No authentication methods can be used. Check your QGIS installation" );

    QgsMessageOutput *output = QgsMessageOutput::createMessageOutput();
    output->setTitle( QObject::tr( "No Authentication Methods" ) );
    output->setMessage( msg, QgsMessageOutput::MessageText );
    output->showMessage();
    return;
  }

  // auth method file regex pattern, only files matching the pattern are loaded if the variable is defined
  const QString filePattern = getenv( "QGIS_AUTHMETHOD_FILE" );
  QRegularExpression fileRegexp;
  if ( !filePattern.isEmpty() )
  {
    fileRegexp.setPattern( filePattern );
  }

  QListIterator<QFileInfo> it( mLibraryDirectory.entryInfoList() );
  while ( it.hasNext() )
  {
    const QFileInfo fi( it.next() );

    if ( !filePattern.isEmpty() )
    {
      if ( fi.fileName().indexOf( fileRegexp ) == -1 )
      {
        QgsDebugMsg( "auth method " + fi.fileName() + " skipped because doesn't match pattern " + filePattern );
        continue;
      }
    }

    QLibrary myLib( fi.filePath() );
    if ( !myLib.load() )
    {
      QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (lib not loadable): %2" ).arg( myLib.fileName(), myLib.errorString() ) );
      continue;
    }

    bool libraryLoaded { false };
    QFunctionPointer func = myLib.resolve( QStringLiteral( "authMethodMetadataFactory" ).toLatin1().data() );
    factory_function *function = reinterpret_cast< factory_function * >( cast_to_fptr( func ) );
    if ( function )
    {
      QgsAuthMethodMetadata *meta = function();
      if ( meta )
      {
        if ( findMetadata_( mAuthMethods, meta->key() ) )
        {
          QgsDebugMsg( QStringLiteral( "Checking %1: ...invalid (key %2 already registered)" ).arg( myLib.fileName() ).arg( meta->key() ) );
          delete meta;
          continue;
        }
        // add this method to the map
        mAuthMethods[meta->key()] = meta;
        libraryLoaded = true;
      }
    }
    if ( ! libraryLoaded )
    {
      QgsDebugMsgLevel( QStringLiteral( "Checking %1: ...invalid (no authMethodMetadataFactory method)" ).arg( myLib.fileName() ), 2 );
    }
  }
#endif
}

// typedef for the unload auth method function
typedef void cleanupAuthMethod_t();

QgsAuthMethodRegistry::~QgsAuthMethodRegistry()
{
  clean();
  if ( sInstance == this )
    sInstance = nullptr;
};

void QgsAuthMethodRegistry::clean()
{
  AuthMethods::const_iterator it = mAuthMethods.begin();

  while ( it != mAuthMethods.end() )
  {
    QgsDebugMsgLevel( QStringLiteral( "cleanup: %1" ).arg( it->first ), 5 );
    const QString lib = it->second->library();
    QLibrary myLib( lib );
    if ( myLib.isLoaded() )
    {
      cleanupAuthMethod_t *cleanupFunc = reinterpret_cast< cleanupAuthMethod_t * >( cast_to_fptr( myLib.resolve( "cleanupAuthMethod" ) ) );
      if ( cleanupFunc )
        cleanupFunc();
    }
    // clear cached QgsAuthMethodMetadata *
    delete it->second;
    ++it;
  }
}


QString QgsAuthMethodRegistry::library( const QString &authMethodKey ) const
{
  QgsAuthMethodMetadata *md = findMetadata_( mAuthMethods, authMethodKey );

  if ( md )
  {
    Q_NOWARN_DEPRECATED_PUSH
    return md->library();
    Q_NOWARN_DEPRECATED_POP
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
    list += QLatin1String( "<ol>" );
  }

  while ( it != mAuthMethods.end() )
  {
    if ( asHtml )
    {
      list += QLatin1String( "<li>" );
    }

    list += it->second->description();

    if ( asHtml )
    {
      list += QLatin1String( "<br></li>" );
    }
    else
    {
      list += '\n';
    }

    ++it;
  }

  if ( asHtml )
  {
    list += QLatin1String( "</ol>" );
  }

  return list;
}

QDir QgsAuthMethodRegistry::libraryDirectory() const
{
  return mLibraryDirectory;
}

void QgsAuthMethodRegistry::setLibraryDirectory( const QDir &path )
{
  mLibraryDirectory = path;
}


// typedef for the QgsAuthMethod class factory
typedef QgsAuthMethod *classFactoryFunction_t();

const QgsAuthMethodMetadata *QgsAuthMethodRegistry::authMethodMetadata( const QString &authMethodKey ) const
{
  return findMetadata_( mAuthMethods, authMethodKey );
}

QgsAuthMethod *QgsAuthMethodRegistry::createAuthMethod( const QString &authMethodKey )
{
  QgsAuthMethodMetadata *metadata = findMetadata_( mAuthMethods, authMethodKey );
  if ( !metadata )
  {
    QgsMessageLog::logMessage( QObject::tr( "Invalid auth method %1" ).arg( authMethodKey ) );
    return nullptr;
  }

  return metadata->createAuthMethod();
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


