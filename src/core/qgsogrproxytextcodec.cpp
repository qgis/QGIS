/***************************************************************************
                             qgsogrproxytextcodec.cpp
                             -------------
    begin                : June 2020
    copyright            : (C) 2020 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsogrproxytextcodec.h"
#include "qgslogger.h"
#include <cpl_string.h>
#include <mutex>

QgsOgrProxyTextCodec::QgsOgrProxyTextCodec( const QByteArray &name )
  : mName( name )
{

}

QString QgsOgrProxyTextCodec::convertToUnicode( const char *chars, int, ConverterState * ) const
{
  if ( !chars )
    return QString();

  char *res = CPLRecode( chars, mName.constData(), CPL_ENC_UTF8 );
  if ( !res )
  {
    QgsDebugError( "convertToUnicode failed" );
    return QString();
  }

  const QString result = QString::fromUtf8( res );
  CPLFree( res );
  return result;
}

QByteArray QgsOgrProxyTextCodec::convertFromUnicode( const QChar *unicode, int length, ConverterState * ) const
{
  if ( !unicode )
    return QByteArray();

  const QString src = QString( unicode, length );
  char *res = CPLRecode( src.toUtf8().constData(), CPL_ENC_UTF8, mName.constData() );
  if ( !res )
  {
    QgsDebugError( "convertFromUnicode failed" );
    return QByteArray();
  }

  const QByteArray result = QByteArray( res );
  CPLFree( res );
  return result;
}

// MY 5 YEAR OLD DAUGHTER WROTE THIS LINE, REMOVE AT YOUR OWN RISK!!!
// i don't want this to be here

QByteArray QgsOgrProxyTextCodec::name() const
{
  return mName;
}

QList<QByteArray> QgsOgrProxyTextCodec::aliases() const
{
  return QList<QByteArray>();
}

int QgsOgrProxyTextCodec::mibEnum() const
{
  // doesn't seem required in this case
  return 0;
}

QStringList QgsOgrProxyTextCodec::supportedCodecs()
{
  static QStringList codecs;
  static std::once_flag initialized;
  std::call_once( initialized, [&]( )
  {
    // there's likely others that are supported by GDAL, but we're primarily concerned here
    // with codecs used by the shapefile driver, and which are no longer supported on the
    // windows Qt builds (due to removal of ICU support in windows Qt builds)
    // see https://github.com/qgis/QGIS/issues/36871
    for ( int i = 437; i <= 950; ++i )
      codecs << QStringLiteral( "CP%1" ).arg( i );
    for ( int i = 1250; i <= 1258; ++i )
      codecs << QStringLiteral( "CP%1" ).arg( i );
    codecs << QStringLiteral( "CP1251" );
  } );
  return codecs;
}
