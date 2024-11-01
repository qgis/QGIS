/***************************************************************************
  qgsserverstatichandler.cpp - QgsServerStaticHandler

 ---------------------
 begin                : 30.7.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsserverstatichandler.h"
#include "qgsmessagelog.h"
#include "qgsserverresponse.h"

#include <QFile>
#include <QMimeDatabase>


QgsServerStaticHandler::QgsServerStaticHandler( const QString &pathRegExp, const QString &staticPathSuffix )
  : mPathRegExp( pathRegExp )
  , mStaticPathSuffix( staticPathSuffix )
{
  setContentTypes( { QgsServerOgcApi::ContentType::HTML } );
}

void QgsServerStaticHandler::handleRequest( const QgsServerApiContext &context ) const
{
  const QRegularExpressionMatch match { path().match( context.request()->url().path() ) };
  if ( !match.hasMatch() )
  {
    throw QgsServerApiNotFoundError( QStringLiteral( "Static file was not found" ) );
  }

  const QString staticFilePath { match.captured( QStringLiteral( "staticFilePath" ) ) };
  // Calculate real path
  QString filePath { staticPath( context ) };
  if ( !mStaticPathSuffix.isEmpty() )
  {
    filePath += '/' + mStaticPathSuffix;
  }
  filePath += '/' + staticFilePath;
  if ( !QFile::exists( filePath ) )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Static file was not found: %1" ).arg( filePath ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    throw QgsServerApiNotFoundError( QStringLiteral( "Static file %1 was not found" ).arg( staticFilePath ) );
  }

  QFile f( filePath );
  if ( !f.open( QIODevice::ReadOnly ) )
  {
    throw QgsServerApiInternalServerError( QStringLiteral( "Could not open static file %1" ).arg( staticFilePath ) );
  }

  const qint64 size { f.size() };
  const QByteArray content { f.readAll() };
  const QMimeType mimeType { QMimeDatabase().mimeTypeForFile( filePath ) };
  context.response()->setHeader( QStringLiteral( "Content-Type" ), mimeType.name() );
  context.response()->setHeader( QStringLiteral( "Content-Length" ), QString::number( size ) );
  context.response()->write( content );
}
