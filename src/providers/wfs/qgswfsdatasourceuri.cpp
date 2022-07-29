/***************************************************************************
    qgswfsdatasourceuri.cpp
    ---------------------
    begin                : February 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "QtGlobal"

#include "qgswfsconstants.h"
#include "qgswfsdatasourceuri.h"
#include "qgsmessagelog.h"

#include <QUrlQuery>

QgsWFSDataSourceURI::QgsWFSDataSourceURI( const QString &uri )
  : mURI( uri )
{
  typedef QPair<QString, QString> queryItem;

  // Compatibility with QGIS < 2.16 layer URI of the format
  // http://example.com/?SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=x&SRSNAME=y&username=foo&password=
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_URL ) )
  {
    mDeprecatedURI = true;
    static const QSet<QString> sFilter
    {
      QStringLiteral( "service" ),
      QgsWFSConstants::URI_PARAM_VERSION,
      QgsWFSConstants::URI_PARAM_TYPENAME,
      QStringLiteral( "request" ),
      QgsWFSConstants::URI_PARAM_BBOX,
      QgsWFSConstants::URI_PARAM_SRSNAME,
      QgsWFSConstants::URI_PARAM_FILTER,
      QgsWFSConstants::URI_PARAM_OUTPUTFORMAT,
      QgsWFSConstants::URI_PARAM_USERNAME,
      QgsWFSConstants::URI_PARAM_PASSWORD,
      QgsWFSConstants::URI_PARAM_AUTHCFG
    };

    QUrl url( uri );
    QUrlQuery query( url );
    // Transform all param keys to lowercase
    const QList<QPair<QString, QString> > items( query.queryItems() );
    for ( const queryItem &item : items )
    {
      query.removeQueryItem( item.first );
      query.addQueryItem( item.first.toLower(), item.second );
    }

    const QString srsname = query.queryItemValue( QgsWFSConstants::URI_PARAM_SRSNAME );
    const QString bbox = query.queryItemValue( QgsWFSConstants::URI_PARAM_BBOX );
    const QString typeName = query.queryItemValue( QgsWFSConstants::URI_PARAM_TYPENAME );
    const QString version = query.queryItemValue( QgsWFSConstants::URI_PARAM_VERSION );
    QString filter = query.queryItemValue( QgsWFSConstants::URI_PARAM_FILTER );
    const QString outputFormat = query.queryItemValue( QgsWFSConstants::URI_PARAM_OUTPUTFORMAT );
    mAuth.mAuthCfg = query.queryItemValue( QgsWFSConstants::URI_PARAM_AUTHCFG );
    // NOTE: A defined authcfg overrides any older username/password auth
    //       Only check for older auth if it is undefined
    if ( mAuth.mAuthCfg.isEmpty() )
    {
      mAuth.mUserName = query.queryItemValue( QgsWFSConstants::URI_PARAM_USERNAME );
      // In QgsDataSourceURI, the "username" param is named "user", check it
      if ( mAuth.mUserName.isEmpty() )
      {
        mAuth.mUserName = query.queryItemValue( QgsWFSConstants::URI_PARAM_USER );
      }
      mAuth.mPassword = query.queryItemValue( QgsWFSConstants::URI_PARAM_PASSWORD );
    }

    // Now remove all stuff that is not the core URL
    for ( const QPair<QString, QString> &param : query.queryItems() )
    {
      if ( sFilter.contains( param.first.toLower() ) )
        query.removeAllQueryItems( param.first );
    }
    url.setQuery( query );

    mURI = QgsDataSourceUri();
    mURI.setParam( QgsWFSConstants::URI_PARAM_URL, url.toEncoded() );
    setTypeName( typeName );
    setSRSName( srsname );
    setVersion( version );
    setOutputFormat( outputFormat );

    //if the xml comes from the dialog, it needs to be a string to pass the validity test
    if ( filter.startsWith( '\'' ) && filter.endsWith( '\'' ) && filter.size() > 1 )
    {
      filter.chop( 1 );
      filter.remove( 0, 1 );
    }

    setFilter( filter );
    if ( !bbox.isEmpty() )
      mURI.setParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX, QStringLiteral( "1" ) );
  }
  else
  {
    QUrl url( mURI.param( QgsWFSConstants::URI_PARAM_URL ) );
    QUrlQuery query( url );
    bool URLModified = false;
    bool somethingChanged = false;
    do
    {
      somethingChanged = false;
      const QList<QPair<QString, QString> > items( query.queryItems() );
      for ( const queryItem &item : items )
      {
        const QString lowerName( item.first.toLower() );
        if ( lowerName == QgsWFSConstants::URI_PARAM_OUTPUTFORMAT )
        {
          setOutputFormat( item.second );
          query.removeQueryItem( item.first );
          somethingChanged = true;
          URLModified = true;
          break;
        }
        else if ( lowerName == QLatin1String( "service" ) ||
                  lowerName == QLatin1String( "request" ) ||
                  lowerName == QLatin1String( "typename" ) ||
                  lowerName == QLatin1String( "typenames" ) ||
                  lowerName == QLatin1String( "version" ) )
        {
          query.removeQueryItem( item.first );
          somethingChanged = true;
          URLModified = true;
          break;
        }
      }
    }
    while ( somethingChanged );
    url.setQuery( query );
    if ( URLModified )
    {
      mURI.setParam( QgsWFSConstants::URI_PARAM_URL, url.toEncoded() );
    }

    mAuth.mUserName = mURI.username();
    mAuth.mPassword = mURI.password();
    mAuth.mAuthCfg = mURI.authConfigId();
  }
}

QSet<QString> QgsWFSDataSourceURI::unknownParamKeys() const
{
  static const QSet<QString> knownKeys
  {
    QgsWFSConstants::URI_PARAM_URL,
    QgsWFSConstants::URI_PARAM_USERNAME,
    QgsWFSConstants::URI_PARAM_USER,
    QgsWFSConstants::URI_PARAM_PASSWORD,
    QgsWFSConstants::URI_PARAM_AUTHCFG,
    QgsWFSConstants::URI_PARAM_VERSION,
    QgsWFSConstants::URI_PARAM_TYPENAME,
    QgsWFSConstants::URI_PARAM_SRSNAME,
    QgsWFSConstants::URI_PARAM_FILTER,
    QgsWFSConstants::URI_PARAM_OUTPUTFORMAT,
    QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX,
    QgsWFSConstants::URI_PARAM_MAXNUMFEATURES,
    QgsWFSConstants::URI_PARAM_IGNOREAXISORIENTATION,
    QgsWFSConstants::URI_PARAM_INVERTAXISORIENTATION,
    QgsWFSConstants::URI_PARAM_VALIDATESQLFUNCTIONS,
    QgsWFSConstants::URI_PARAM_HIDEDOWNLOADPROGRESSDIALOG,
    QgsWFSConstants::URI_PARAM_PAGING_ENABLED,
    QgsWFSConstants::URI_PARAM_PAGE_SIZE,
    QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES
  };

  QSet<QString> l_unknownParamKeys;
  for ( const QString &key : mURI.parameterKeys() )
  {
    if ( !knownKeys.contains( key ) && !( mDeprecatedURI && key == QgsWFSConstants::URI_PARAM_BBOX ) )
    {
      l_unknownParamKeys.insert( key );
    }
  }
  return l_unknownParamKeys;
}

QString QgsWFSDataSourceURI::uri( bool expandAuthConfig ) const
{
  QgsDataSourceUri theURI( mURI );
  // Add authcfg param back into the uri (must be non-empty value)
  if ( ! mAuth.mAuthCfg.isEmpty() )
  {
    theURI.setAuthConfigId( mAuth.mAuthCfg );
  }
  else
  {
    // Add any older username/password auth params back in (allow empty values)
    if ( ! mAuth.mUserName.isNull() )
    {
      theURI.setUsername( mAuth.mUserName );
    }
    if ( ! mAuth.mPassword.isNull() )
    {
      theURI.setPassword( mAuth.mPassword );
    }
  }
  return theURI.uri( expandAuthConfig );
}


QUrl QgsWFSDataSourceURI::baseURL( bool bIncludeServiceWFS ) const
{
  QUrl url( mURI.param( QgsWFSConstants::URI_PARAM_URL ) );
  QUrlQuery query( url );
  if ( bIncludeServiceWFS )
  {
    query.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WFS" ) );
  }
  url.setQuery( query );
  return url;
}

QUrl QgsWFSDataSourceURI::requestUrl( const QString &request, const Method &method ) const
{
  QUrl url;
  QUrlQuery urlQuery;
  switch ( method )
  {
    case Post:
      url = QUrl( mPostEndpoints.contains( request ) ?
                  mPostEndpoints[ request ] : mURI.param( QgsWFSConstants::URI_PARAM_URL ) );
      urlQuery = QUrlQuery( url );
      break;
    default:
    case Get:
    {
      const auto defaultUrl( QUrl( mURI.param( QgsWFSConstants::URI_PARAM_URL ) ) );
      if ( mGetEndpoints.contains( request ) )
      {
        // If the input URL has query parameters, and those are not found in the
        // DCP endpoint, then re-inject them.
        // I'm not completely sure this is the job of the client to do that.
        // One could argue that the server should expose them in the DCP endpoint.
        url = QUrl( mGetEndpoints[ request ] );
        urlQuery = QUrlQuery( url );
        // OGC case insensitive keys KVP: see https://github.com/qgis/QGIS/issues/34148
        QSet<QString> upperCaseQueryItemKeys;
        const QList<QPair<QString, QString>> constQueryItems { urlQuery.queryItems() };
        for ( const auto &qi : constQueryItems )
        {
          upperCaseQueryItemKeys.insert( qi.first.toUpper() );
        }
        const QUrlQuery defaultUrlQuery( defaultUrl );
        const auto itemsDefaultUrl( defaultUrlQuery.queryItems() );
        for ( const auto &item : itemsDefaultUrl )
        {
          if ( !upperCaseQueryItemKeys.contains( item.first.toUpper() ) )
          {
            urlQuery.addQueryItem( item.first, item.second );
          }
        }
      }
      else
      {
        url = defaultUrl;
        urlQuery = QUrlQuery( url );
      }
      break;
    }
  }
  urlQuery.addQueryItem( QStringLiteral( "SERVICE" ), QStringLiteral( "WFS" ) );
  if ( method == Method::Get && ! request.isEmpty() )
    urlQuery.addQueryItem( QStringLiteral( "REQUEST" ), request );
  url.setQuery( urlQuery );
  return url;
}

QString QgsWFSDataSourceURI::version() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_VERSION ) )
    return QgsWFSConstants::VERSION_AUTO;
  return mURI.param( QgsWFSConstants::URI_PARAM_VERSION );
}

long long QgsWFSDataSourceURI::maxNumFeatures() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ) )
    return 0;
  return mURI.param( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ).toLongLong();
}

void QgsWFSDataSourceURI::setMaxNumFeatures( long long maxNumFeatures )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES );
  mURI.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES, QString::number( maxNumFeatures ) );
}

long long QgsWFSDataSourceURI::pageSize() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_PAGE_SIZE ) )
    return 0;
  return mURI.param( QgsWFSConstants::URI_PARAM_PAGE_SIZE ).toLongLong();
}

bool QgsWFSDataSourceURI::pagingEnabled() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_PAGING_ENABLED ) )
    return true;
  return mURI.param( QgsWFSConstants::URI_PARAM_PAGING_ENABLED ) == QLatin1String( "true" );
}

void QgsWFSDataSourceURI::setTypeName( const QString &typeName )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_TYPENAME );
  mURI.setParam( QgsWFSConstants::URI_PARAM_TYPENAME, typeName );
}

QString QgsWFSDataSourceURI::typeName() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_TYPENAME );
}

void QgsWFSDataSourceURI::setSRSName( const QString &crsString )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_SRSNAME );
  if ( !crsString.isEmpty() )
    mURI.setParam( QgsWFSConstants::URI_PARAM_SRSNAME, crsString );
}

void QgsWFSDataSourceURI::setVersion( const QString &versionString )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_VERSION );
  if ( !versionString.isEmpty() )
    mURI.setParam( QgsWFSConstants::URI_PARAM_VERSION, versionString );
}

QString QgsWFSDataSourceURI::SRSName() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_SRSNAME );
}

QString QgsWFSDataSourceURI::filter() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_FILTER );
}

void QgsWFSDataSourceURI::setFilter( const QString &filter )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_FILTER );
  if ( !filter.isEmpty() )
  {
    mURI.setParam( QgsWFSConstants::URI_PARAM_FILTER, filter );
  }
}

QString QgsWFSDataSourceURI::sql() const
{
  return mURI.sql();
}

void QgsWFSDataSourceURI::setSql( const QString &sql )
{
  mURI.setSql( sql );
}

QString QgsWFSDataSourceURI::outputFormat() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_OUTPUTFORMAT );
}

void QgsWFSDataSourceURI::setOutputFormat( const QString &outputFormat )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_OUTPUTFORMAT );
  if ( !outputFormat.isEmpty() )
    mURI.setParam( QgsWFSConstants::URI_PARAM_OUTPUTFORMAT, outputFormat );
}

bool QgsWFSDataSourceURI::isRestrictedToRequestBBOX() const
{
  if ( mURI.hasParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX ) &&
       mURI.param( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX ).toInt() == 1 )
    return true;

  // accept previously used version with typo
  if ( mURI.hasParam( QStringLiteral( "retrictToRequestBBOX" ) ) && mURI.param( QStringLiteral( "retrictToRequestBBOX" ) ).toInt() == 1 )
    return true;

  return false;
}

bool QgsWFSDataSourceURI::ignoreAxisOrientation() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_IGNOREAXISORIENTATION );
}

bool QgsWFSDataSourceURI::invertAxisOrientation() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_INVERTAXISORIENTATION );
}

bool QgsWFSDataSourceURI::validateSqlFunctions() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_VALIDATESQLFUNCTIONS );
}

bool QgsWFSDataSourceURI::hideDownloadProgressDialog() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_HIDEDOWNLOADPROGRESSDIALOG );
}


bool QgsWFSDataSourceURI::preferCoordinatesForWfst11() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES ) &&
         mURI.param( QgsWFSConstants::URI_PARAM_WFST_1_1_PREFER_COORDINATES ).toUpper() == QLatin1String( "TRUE" );
}

QString QgsWFSDataSourceURI::build( const QString &baseUri,
                                    const QString &typeName,
                                    const QString &crsString,
                                    const QString &sql,
                                    const QString &filter,
                                    bool restrictToCurrentViewExtent )
{
  QgsWFSDataSourceURI uri( baseUri );
  uri.setTypeName( typeName );
  uri.setSRSName( crsString );
  uri.setSql( sql );
  uri.setFilter( filter );
  if ( restrictToCurrentViewExtent )
    uri.mURI.setParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX, QStringLiteral( "1" ) );
  if ( uri.version() == QLatin1String( "OGC_API_FEATURES" ) )
  {
    uri.setVersion( QString() );
  }
  return uri.uri();
}

void QgsWFSDataSourceURI::setGetEndpoints( const QgsStringMap &map )
{
  mGetEndpoints = map;
}

void QgsWFSDataSourceURI::setPostEndpoints( const QgsStringMap &map )
{
  mPostEndpoints = map;
}
