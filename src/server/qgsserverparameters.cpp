/***************************************************************************
                              qgsserverparameters.cpp
                              --------------------
  begin                : Jun 27, 2018
  copyright            : (C) 2018 by Paul Blottiere
  email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsserverparameters.h"
#include "qgsserverexception.h"
#include "qgsnetworkcontentfetcher.h"
#include "qgsmessagelog.h"
#include <QObject>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QEventLoop>

//
// QgsServerParameterDefinition
//
QgsServerParameterDefinition::QgsServerParameterDefinition( const QVariant::Type type,
    const QVariant defaultValue )
  : mType( type )
  , mDefaultValue( defaultValue )
{
}

QString QgsServerParameterDefinition::typeName() const
{
  return  QVariant::typeToName( mType );
}

QColor QgsServerParameterDefinition::toColor( bool &ok ) const
{
  ok = true;
  QColor color = mDefaultValue.value<QColor>();
  QString cStr = mValue.toString();

  if ( !cStr.isEmpty() )
  {
    // support hexadecimal notation to define colors
    if ( cStr.startsWith( QLatin1String( "0x" ), Qt::CaseInsensitive ) )
    {
      cStr.replace( 0, 2, QStringLiteral( "#" ) );
    }

    color = QColor( cStr );

    ok = color.isValid();
  }

  return color;
}

QString QgsServerParameterDefinition::toString( const bool defaultValue ) const
{
  QString value = mValue.toString();

  if ( value.isEmpty() && defaultValue )
    value = mDefaultValue.toString();

  return value;
}

QStringList QgsServerParameterDefinition::toStringList( const char delimiter, const bool skipEmptyParts ) const
{
  if ( skipEmptyParts )
  {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    return toString().split( delimiter, QString::SkipEmptyParts );
#else
    return toString().split( delimiter, Qt::SkipEmptyParts );
#endif
  }
  else
  {
    QStringList list;
    if ( !toString().isEmpty() )
    {
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
      list = toString().split( delimiter, QString::KeepEmptyParts );
#else
      list = toString().split( delimiter, Qt::KeepEmptyParts );
#endif
    }
    return list;
  }
}

QList<QgsGeometry> QgsServerParameterDefinition::toGeomList( bool &ok, const char delimiter ) const
{
  ok = true;
  QList<QgsGeometry> geoms;

  const auto constStringList( toStringList( delimiter ) );
  for ( const auto &wkt : constStringList )
  {
    const QgsGeometry g( QgsGeometry::fromWkt( wkt ) );

    if ( g.isGeosValid() )
    {
      geoms.append( g );
    }
    else
    {
      ok = false;
      return QList<QgsGeometry>();
    }
  }

  return geoms;
}

QStringList QgsServerParameterDefinition::toOgcFilterList() const
{
  int pos = 0;
  QStringList filters;
  const QString filter = toString();

  while ( pos < filter.size() )
  {
    if ( pos + 1 < filter.size() && filter[pos] == '(' && filter[pos + 1] == '<' )
    {
      // OGC filter on multiple layers
      int posEnd = filter.indexOf( "Filter>)", pos );
      if ( posEnd < 0 )
      {
        posEnd = filter.size();
      }
      filters.append( filter.mid( pos + 1, posEnd - pos + 6 ) );
      pos = posEnd + 8;
    }
    else if ( pos + 1 < filter.size() && filter[pos] == '(' && filter[pos + 1] == ')' )
    {
      // empty OGC filter
      filters.append( "" );
      pos += 2;
    }
    else if ( filter[pos] == '<' && pos + 7 < filter.size() && filter.mid( pos + 1, 6 ).compare( QLatin1String( "Filter" ) ) == 0 )
    {
      // Single OGC filter
      filters.append( filter.mid( pos ) );
      break;
    }
    else
    {
      pos += 1;
    }
  }

  return filters;
}

QStringList QgsServerParameterDefinition::toExpressionList() const
{
  int pos = 0;
  QStringList filters;
  const QString filter = toString();

  auto isOgcFilter = [filter]()
  {
    return filter.contains( QStringLiteral( "<Filter>" ) ) or filter.contains( QStringLiteral( "()" ) );
  };

  while ( pos < filter.size() )
  {
    int posEnd = filter.indexOf( ';', pos );

    if ( posEnd == pos + 1 )
    {
      if ( ! isOgcFilter() )
        filters.append( QString() );
      pos = posEnd;
      continue;
    }

    if ( ! isOgcFilter() )
      filters.append( filter.mid( pos, posEnd - pos ) );

    if ( posEnd < 0 )
    {
      pos = filter.size();
    }
    else
    {
      pos = posEnd + 1;
    }
  }

  if ( ! filter.isEmpty() && filter.back() == ';' )
  {
    filters.append( QString() );
  }

  return filters;
}

QList<QColor> QgsServerParameterDefinition::toColorList( bool &ok, const char delimiter ) const
{
  ok = true;
  QList<QColor> colors;

  const auto constStringList( toStringList( delimiter ) );
  for ( const auto &part : constStringList )
  {
    QString cStr( part );
    if ( !cStr.isEmpty() )
    {
      // support hexadecimal notation to define colors
      if ( cStr.startsWith( QLatin1String( "0x" ), Qt::CaseInsensitive ) )
      {
        cStr.replace( 0, 2, QStringLiteral( "#" ) );
      }

      const QColor color = QColor( cStr );
      ok = color.isValid();

      if ( !ok )
      {
        return QList<QColor>();
      }

      colors.append( color );
    }
  }

  return colors;
}

QList<int> QgsServerParameterDefinition::toIntList( bool &ok, const char delimiter ) const
{
  ok = true;
  QList<int> ints;

  const auto constStringList( toStringList( delimiter ) );
  for ( const auto &part : constStringList )
  {
    const int val = part.toInt( &ok );

    if ( !ok )
    {
      return QList<int>();
    }

    ints.append( val );
  }

  return ints;
}

QList<double> QgsServerParameterDefinition::toDoubleList( bool &ok, const char delimiter ) const
{
  ok = true;
  QList<double> vals;

  const auto constStringList( toStringList( delimiter ) );
  for ( const auto &part : constStringList )
  {
    const double val = part.toDouble( &ok );

    if ( !ok )
    {
      return QList<double>();
    }

    vals.append( val );
  }

  return vals;
}

QgsRectangle QgsServerParameterDefinition::toRectangle( bool &ok ) const
{
  ok = true;
  QgsRectangle extent;

  if ( !mValue.toString().isEmpty() )
  {
    QStringList corners = mValue.toString().split( ',' );

    if ( corners.size() == 4 )
    {
      double d[4];

      for ( int i = 0; i < 4; i++ )
      {
        corners[i].replace( ' ', '+' );
        d[i] = corners[i].toDouble( &ok );
        if ( !ok )
        {
          return QgsRectangle();
        }
      }

      if ( d[0] > d[2] || d[1] > d[3] )
      {
        ok = false;
        return QgsRectangle();
      }

      extent = QgsRectangle( d[0], d[1], d[2], d[3] );
    }
    else
    {
      ok = false;
      return QgsRectangle();
    }
  }

  return extent;
}

QString QgsServerParameterDefinition::loadUrl( bool &ok ) const
{
  ok = true;

  // Get URL
  const QUrl url = toUrl( ok );
  if ( !ok )
  {
    return QString();
  }

  // fetching content
  QgsNetworkContentFetcher fetcher;
  QEventLoop loop;
  QObject::connect( &fetcher, &QgsNetworkContentFetcher::finished, &loop, &QEventLoop::quit );

  QgsMessageLog::logMessage(
    QObject::tr( "Request started [url: %1]" ).arg( url.toString() ),
    QStringLiteral( "Server" ) );
  QNetworkRequest request( url );
  request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  fetcher.fetchContent( request );

  //wait until content fetched
  loop.exec( QEventLoop::ExcludeUserInputEvents );

  QNetworkReply *reply = fetcher.reply();
  if ( !reply )
  {
    ok = false;
    QgsMessageLog::logMessage(
      QObject::tr( "Request failed [error: no reply - url: %1]" ).arg( url.toString() ),
      QStringLiteral( "Server" ) );
    return QString();
  }

  const QVariant status = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
  if ( !status.isNull() && status.toInt() >= 400 )
  {
    ok = false;
    if ( reply->error() != QNetworkReply::NoError )
    {
      QgsMessageLog::logMessage(
        QObject::tr( "Request failed [error: %1 - url: %2]" ).arg( reply->errorString(), reply->url().toString() ),
        QStringLiteral( "Server" ) );
    }
    const QVariant phrase = reply->attribute( QNetworkRequest::HttpReasonPhraseAttribute );
    QgsMessageLog::logMessage(
      QObject::tr( "Request error [status: %1 - reason phrase: %2] for %3" ).arg( status.toInt() ).arg( phrase.toString(), reply->url().toString() ),
      QStringLiteral( "Server" ) );
    return QString();
  }

  if ( reply->error() != QNetworkReply::NoError )
  {
    ok = false;
    QgsMessageLog::logMessage(
      QObject::tr( "Request failed [error: %1 - url: %2]" ).arg( reply->errorString(), reply->url().toString() ),
      QStringLiteral( "Server" ) );
    return QString();
  }

  QgsMessageLog::logMessage(
    QObject::tr( "Request finished [url: %1]" ).arg( url.toString() ),
    QStringLiteral( "Server" ) );

  QString content = fetcher.contentAsString();
  ok = ( !content.isEmpty() );
  return content;
}

QUrl QgsServerParameterDefinition::toUrl( bool &ok ) const
{
  ok = true;
  QUrl val;

  if ( !mValue.toString().isEmpty() )
  {
    val = mValue.toUrl();
  }

  ok = ( !val.isEmpty() && val.isValid() );
  return val;
}

int QgsServerParameterDefinition::toInt( bool &ok ) const
{
  ok = true;
  int val = mDefaultValue.toInt();

  if ( !mValue.toString().isEmpty() )
  {
    val = mValue.toInt( &ok );
  }

  return val;
}

bool QgsServerParameterDefinition::toBool() const
{
  int val = mDefaultValue.toBool();

  if ( !mValue.toString().isEmpty() )
  {
    val = mValue.toBool();
  }

  return val;
}

double QgsServerParameterDefinition::toDouble( bool &ok ) const
{
  ok = true;
  double val = mDefaultValue.toDouble();

  if ( !mValue.toString().isEmpty() )
  {
    val = mValue.toDouble( &ok );
  }

  return val;
}

bool QgsServerParameterDefinition::isValid() const
{
  return mValue.canConvert( mType );
}

void QgsServerParameterDefinition::raiseError( const QString &msg )
{
  throw QgsBadRequestException( QStringLiteral( "Invalid Parameter" ), msg );
}

//
// QgsServerParameter
//
QgsServerParameter::QgsServerParameter( const QgsServerParameter::Name name,
                                        const QVariant::Type type, const QVariant defaultValue )
  : QgsServerParameterDefinition( type, defaultValue )
  , mName( name )
{
}

QString QgsServerParameter::name( const QgsServerParameter::Name name )
{
  if ( name == QgsServerParameter::VERSION_SERVICE )
  {
    return QStringLiteral( "VERSION" );
  }
  else
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerParameter::Name>() );
    return metaEnum.valueToKey( name );
  }
}

QgsServerParameter::Name QgsServerParameter::name( const QString &name )
{
  if ( name.compare( QLatin1String( "VERSION" ) ) == 0 )
  {
    return QgsServerParameter::VERSION_SERVICE;
  }
  else
  {
    const QMetaEnum metaEnum( QMetaEnum::fromType<QgsServerParameter::Name>() );
    return ( QgsServerParameter::Name ) metaEnum.keyToValue( name.toUpper().toStdString().c_str() );
  }
}

void QgsServerParameter::raiseError() const
{
  const QString msg = QString( "%1 ('%2') cannot be converted into %3" ).arg( name( mName ), mValue.toString(), typeName() );
  QgsServerParameterDefinition::raiseError( msg );
}

//
// QgsServerParameters
//
QgsServerParameters::QgsServerParameters()
{
  save( QgsServerParameter( QgsServerParameter::SERVICE ) );
  save( QgsServerParameter( QgsServerParameter::REQUEST ) );
  save( QgsServerParameter( QgsServerParameter::VERSION_SERVICE ) );
  save( QgsServerParameter( QgsServerParameter::MAP ) );
  save( QgsServerParameter( QgsServerParameter::FILE_NAME ) );
}

QgsServerParameters::QgsServerParameters( const QUrlQuery &query )
  : QgsServerParameters()
{
  mUrlQuery = query;
  load( query );
}

void QgsServerParameters::save( const QgsServerParameter &parameter )
{
  mParameters[ parameter.mName ] = parameter;
}

void QgsServerParameters::add( const QString &key, const QString &value )
{
  QUrlQuery query;
  query.addQueryItem( key, value );
  load( query );
}

QUrlQuery QgsServerParameters::urlQuery() const
{
  QUrlQuery query = mUrlQuery;

  if ( query.isEmpty() )
  {
    query.clear();

    const auto constMap( toMap().toStdMap() );
    for ( const auto &param : constMap )
    {
      const QString value = QUrl::toPercentEncoding( QString( param.second ) );
      query.addQueryItem( param.first, value );
    }
  }

  return query;
}

void QgsServerParameters::remove( QgsServerParameter::Name name )
{
  remove( QgsServerParameter::name( name ) );
}

void QgsServerParameters::remove( const QString &key )
{
  if ( mUnmanagedParameters.contains( key ) )
  {
    mUnmanagedParameters.take( key );
  }
  else
  {
    const QgsServerParameter::Name paramName = QgsServerParameter::name( key );
    if ( mParameters.contains( paramName ) )
    {
      mParameters.take( paramName );
    }
  }
}

QString QgsServerParameters::map() const
{
  return value( QgsServerParameter::MAP ).toString();
}

QString QgsServerParameters::version() const
{
  return value( QgsServerParameter::VERSION_SERVICE ).toString();
}

QString QgsServerParameters::fileName() const
{
  return value( QgsServerParameter::FILE_NAME ).toString();
}

QString QgsServerParameters::service() const
{
  QString serviceValue = value( QgsServerParameter::SERVICE ).toString();

  if ( serviceValue.isEmpty() )
  {
    // SERVICE not mandatory for WMS 1.3.0 GetMap & GetFeatureInfo
    if ( request() == QLatin1String( "GetMap" ) \
         || request() == QLatin1String( "GetFeatureInfo" ) )
    {
      serviceValue = "WMS";
    }
  }

  return serviceValue;
}

QMap<QString, QString> QgsServerParameters::toMap() const
{
  QMap<QString, QString> params = mUnmanagedParameters;

  for ( const auto &parameter : mParameters.toStdMap() )
  {
    if ( parameter.second.mValue.isNull() )
      continue;

    if ( parameter.second.mName == QgsServerParameter::VERSION_SERVICE )
    {
      params["VERSION"] = parameter.second.mValue.toString();
    }
    else
    {
      const QString paramName = QgsServerParameter::name( parameter.first );
      params[paramName] = parameter.second.mValue.toString();
    }
  }

  return params;
}

QString QgsServerParameters::request() const
{
  return value( QgsServerParameter::REQUEST ).toString();
}

QString QgsServerParameters::value( const QString &key ) const
{
  if ( ! mParameters.contains( QgsServerParameter::name( key ) ) )
  {
    return mUnmanagedParameters[key];
  }
  else
  {
    return value( QgsServerParameter::name( key ) ).toString();
  }
}

QVariant QgsServerParameters::value( QgsServerParameter::Name name ) const
{
  return mParameters[name].mValue;
}

void QgsServerParameters::load( const QUrlQuery &query )
{
  // clean query string first
  QUrlQuery cleanQuery( query );
  cleanQuery.setQuery( query.query().replace( '+', QLatin1String( "%20" ) ) );

  // load parameters
  const auto constQueryItems( cleanQuery.queryItems( QUrl::FullyDecoded ) );
  for ( const auto &item : constQueryItems )
  {
    const QgsServerParameter::Name name = QgsServerParameter::name( item.first );
    if ( name >= 0 )
    {
      mParameters[name].mValue = item.second;
      if ( ! mParameters[name].isValid() )
      {
        mParameters[name].raiseError();
      }
    }
    else if ( item.first.compare( QLatin1String( "VERSION" ),  Qt::CaseInsensitive ) == 0 )
    {
      const QgsServerParameter::Name name = QgsServerParameter::VERSION_SERVICE;
      mParameters[name].mValue = item.second;
      if ( ! mParameters[name].isValid() )
      {
        mParameters[name].raiseError();
      }
    }
    else if ( ! loadParameter( item.first, item.second ) )
    {
      mUnmanagedParameters[item.first.toUpper()] = item.second;
    }
  }
}

bool QgsServerParameters::loadParameter( const QString &, const QString & )
{
  return false;
}

void QgsServerParameters::clear()
{
  mParameters.clear();
  mUnmanagedParameters.clear();
}
