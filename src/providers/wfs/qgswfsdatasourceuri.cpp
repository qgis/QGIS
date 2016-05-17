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

#include "qgswfsconstants.h"
#include "qgswfsdatasourceuri.h"
#include "qgsmessagelog.h"

QgsWFSDataSourceURI::QgsWFSDataSourceURI( const QString& uri )
    : mURI( uri )
{
  // Compatiblity with QGIS < 2.16 layer URI of the format
  // http://example.com/?SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=x&SRSNAME=y&username=foo&password=
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_URL ) )
  {
    QUrl url( uri );
    QString srsname = url.queryItemValue( "SRSNAME" );
    QString bbox = url.queryItemValue( "BBOX" );
    QString typeName = url.queryItemValue( "TYPENAME" );
    QString filter = url.queryItemValue( "FILTER" );
    mAuth.mUserName = url.queryItemValue( QgsWFSConstants::URI_PARAM_USERNAME );
    mAuth.mPassword = url.queryItemValue( QgsWFSConstants::URI_PARAM_PASSWORD );
    mAuth.mAuthCfg = url.queryItemValue( QgsWFSConstants::URI_PARAM_AUTHCFG );

    // Now remove all stuff that is not the core URL
    url.removeQueryItem( "SERVICE" );
    url.removeQueryItem( "VERSION" );
    url.removeQueryItem( "TYPENAME" );
    url.removeQueryItem( "REQUEST" );
    url.removeQueryItem( "BBOX" );
    url.removeQueryItem( "SRSNAME" );
    url.removeQueryItem( "FILTER" );
    url.removeQueryItem( QgsWFSConstants::URI_PARAM_USERNAME );
    url.removeQueryItem( QgsWFSConstants::URI_PARAM_PASSWORD );
    url.removeQueryItem( QgsWFSConstants::URI_PARAM_AUTHCFG );

    mURI = QgsDataSourceURI();
    mURI.setParam( QgsWFSConstants::URI_PARAM_URL, url.toEncoded() );
    setTypeName( typeName );
    setSRSName( srsname );

    //if the xml comes from the dialog, it needs to be a string to pass the validity test
    if ( filter.startsWith( '\'' ) && filter.endsWith( '\'' ) && filter.size() > 1 )
    {
      filter.chop( 1 );
      filter.remove( 0, 1 );
    }

    setFilter( filter );
    if ( !bbox.isEmpty() )
      mURI.setParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX, "1" );
  }
  else
  {
    mAuth.mUserName = mURI.param( QgsWFSConstants::URI_PARAM_USERNAME );
    mAuth.mPassword = mURI.param( QgsWFSConstants::URI_PARAM_PASSWORD );
    mAuth.mAuthCfg = mURI.param( QgsWFSConstants::URI_PARAM_AUTHCFG );
  }
}

QString QgsWFSDataSourceURI::uri()
{
  return mURI.uri();
}

QUrl QgsWFSDataSourceURI::baseURL( bool bIncludeServiceWFS ) const
{
  QUrl url( mURI.param( QgsWFSConstants::URI_PARAM_URL ) );
  if ( bIncludeServiceWFS )
  {
    url.addQueryItem( "SERVICE", "WFS" );
  }
  return url;
}

QString QgsWFSDataSourceURI::version() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_VERSION ) )
    return QgsWFSConstants::VERSION_AUTO;
  return mURI.param( QgsWFSConstants::URI_PARAM_VERSION );
}

int QgsWFSDataSourceURI::maxNumFeatures() const
{
  if ( !mURI.hasParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ) )
    return 0;
  return mURI.param( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES ).toInt();
}

void QgsWFSDataSourceURI::setMaxNumFeatures( int maxNumFeatures )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES );
  mURI.setParam( QgsWFSConstants::URI_PARAM_MAXNUMFEATURES , QString( maxNumFeatures ) );
}

void QgsWFSDataSourceURI::setTypeName( const QString& typeName )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_TYPENAME );
  mURI.setParam( QgsWFSConstants::URI_PARAM_TYPENAME, typeName );
}

QString QgsWFSDataSourceURI::typeName() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_TYPENAME );
}

void QgsWFSDataSourceURI::setSRSName( const QString& crsString )
{
  mURI.removeParam( QgsWFSConstants::URI_PARAM_SRSNAME );
  if ( !crsString.isEmpty() )
    mURI.setParam( QgsWFSConstants::URI_PARAM_SRSNAME, crsString );
}

QString QgsWFSDataSourceURI::SRSName() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_SRSNAME );
}

QString QgsWFSDataSourceURI::filter() const
{
  return mURI.param( QgsWFSConstants::URI_PARAM_FILTER );
}

void QgsWFSDataSourceURI::setFilter( const QString& filter )
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

void QgsWFSDataSourceURI::setSql( const QString& sql )
{
  mURI.setSql( sql );
}

bool QgsWFSDataSourceURI::isRestrictedToRequestBBOX() const
{
  return mURI.hasParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX ) &&
         mURI.param( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX ).toInt() == 1;
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

QString QgsWFSDataSourceURI::build( const QString& baseUri,
                                    const QString& typeName,
                                    const QString& crsString,
                                    const QString& sql,
                                    bool restrictToCurrentViewExtent )
{
  QgsWFSDataSourceURI uri( baseUri );
  uri.setTypeName( typeName );
  uri.setSRSName( crsString );
  uri.setSql( sql );
  if ( restrictToCurrentViewExtent )
    uri.mURI.setParam( QgsWFSConstants::URI_PARAM_RESTRICT_TO_REQUEST_BBOX, "1" );
  return uri.uri();
}
