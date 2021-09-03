/***************************************************************************
    qgscplhttpfetchoverrider.cpp
    ----------------------------
    begin                : September 2020
    copyright            : (C) 2020 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscplhttpfetchoverrider.h"
#include "qgslogger.h"
#include "qgsblockingnetworkrequest.h"

#include "cpl_http.h"
#include "gdal.h"

#if GDAL_VERSION_NUM < GDAL_COMPUTE_VERSION(3,2,0)

QgsCPLHTTPFetchOverrider::QgsCPLHTTPFetchOverrider( const QString &authCfg, QgsFeedback *feedback )
{
  Q_UNUSED( authCfg );
  Q_UNUSED( feedback );
  Q_UNUSED( mAuthCfg );
  Q_UNUSED( mFeedback );
}

QgsCPLHTTPFetchOverrider::~QgsCPLHTTPFetchOverrider()
{
}

#else

QgsCPLHTTPFetchOverrider::QgsCPLHTTPFetchOverrider( const QString &authCfg, QgsFeedback *feedback ):
  mAuthCfg( authCfg ),
  mFeedback( feedback )
{
  CPLHTTPPushFetchCallback( QgsCPLHTTPFetchOverrider::callback, this );
}

QgsCPLHTTPFetchOverrider::~QgsCPLHTTPFetchOverrider()
{
  CPLHTTPPopFetchCallback();
}


CPLHTTPResult *QgsCPLHTTPFetchOverrider::callback( const char *pszURL,
    CSLConstList papszOptions,
    GDALProgressFunc /* pfnProgress */,
    void * /*pProgressArg */,
    CPLHTTPFetchWriteFunc pfnWrite,
    void *pWriteArg,
    void *pUserData )
{
  QgsCPLHTTPFetchOverrider *pThis = static_cast<QgsCPLHTTPFetchOverrider *>( pUserData );

  auto psResult = static_cast<CPLHTTPResult *>( CPLCalloc( sizeof( CPLHTTPResult ), 1 ) );
  if ( CSLFetchNameValue( papszOptions, "CLOSE_PERSISTENT" ) )
  {
    // CLOSE_PERSISTENT is a CPL trick to maintain a curl handle open over
    // a series of CPLHTTPFetch() call to the same server.
    // Just return a dummy result to acknowledge we 'processed' it
    return psResult;
  }

  // Look for important options we don't handle yet
  for ( const char *pszOption : { "FORM_FILE_PATH", "FORM_ITEM_COUNT" } )
  {
    if ( CSLFetchNameValue( papszOptions, pszOption ) )
    {
      QgsDebugMsg( QStringLiteral( "Option %1 not handled" ).arg( pszOption ) );
      return nullptr;
    }
  }

  QgsBlockingNetworkRequest blockingRequest;
  blockingRequest.setAuthCfg( pThis->mAuthCfg );

  QNetworkRequest request( QString::fromUtf8( pszURL ) );
  for ( const auto &keyValue : pThis->mAttributes )
  {
    request.setAttribute( keyValue.first, keyValue.second );
  }

  // Store request headers
  const char *pszHeaders = CSLFetchNameValue( papszOptions, "HEADERS" );
  if ( pszHeaders )
  {
    char **papszTokensHeaders = CSLTokenizeString2( pszHeaders, "\r\n", 0 );
    for ( int i = 0; papszTokensHeaders[i] != nullptr; ++i )
    {
      char *pszKey = nullptr;
      const char *pszValue = CPLParseNameValue( papszTokensHeaders[i], &pszKey );
      if ( pszKey && pszValue )
      {
        request.setRawHeader(
          QByteArray::fromStdString( pszKey ),
          QByteArray::fromStdString( pszValue ) );
      }
      CPLFree( pszKey );
    }
    CSLDestroy( papszTokensHeaders );
  }

  constexpr bool forceRefresh = true;
  const char *pszCustomRequest = CSLFetchNameValue( papszOptions, "CUSTOMREQUEST" );
  const char *pszPostFields = CSLFetchNameValue( papszOptions, "POSTFIELDS" );
  QgsBlockingNetworkRequest::ErrorCode errCode;
  if ( pszPostFields )
  {
    if ( pszCustomRequest == nullptr || EQUAL( pszCustomRequest, "POST" ) )
    {
      errCode = blockingRequest.post( request,
                                      QByteArray::fromStdString( pszPostFields ),
                                      forceRefresh,
                                      pThis->mFeedback );
    }
    else if ( EQUAL( pszCustomRequest, "PUT" ) )
    {
      errCode = blockingRequest.put( request,
                                     QByteArray::fromStdString( pszPostFields ),
                                     pThis->mFeedback );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Invalid CUSTOMREQUEST = %1 when POSTFIELDS is defined" ).arg( pszCustomRequest ) );
      return nullptr;
    }
  }
  else
  {
    if ( pszCustomRequest == nullptr || EQUAL( pszCustomRequest, "GET" ) )
    {
      errCode = blockingRequest.get( request, forceRefresh, pThis->mFeedback );
    }
    else if ( EQUAL( pszCustomRequest, "HEAD" ) )
    {
      errCode = blockingRequest.head( request, forceRefresh, pThis->mFeedback );
    }
    else if ( EQUAL( pszCustomRequest, "DELETE" ) )
    {
      errCode = blockingRequest.deleteResource( request, pThis->mFeedback );
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "Invalid CUSTOMREQUEST = %1 when POSTFIELDS is not defined" ).arg( pszCustomRequest ) );
      return nullptr;
    }
  }
  if ( errCode != QgsBlockingNetworkRequest::NoError )
  {
    psResult->nStatus = 1;
    psResult->pszErrBuf = CPLStrdup( blockingRequest.errorMessage().toUtf8() );
    return psResult;
  }

  const QgsNetworkReplyContent reply( blockingRequest.reply() );

  // Store response headers
  for ( const auto &pair : reply.rawHeaderPairs() )
  {
    if ( EQUAL( pair.first.toStdString().c_str(), "Content-Type" ) )
    {
      CPLFree( psResult->pszContentType );
      psResult->pszContentType = CPLStrdup( pair.second.toStdString().c_str() );
    }
    psResult->papszHeaders = CSLAddNameValue(
                               psResult->papszHeaders,
                               pair.first.toStdString().c_str(),
                               pair.second.toStdString().c_str() );
  }

  // Process content
  QByteArray content( reply.content() );

  // Poor-man implementation of the pfnWrite mechanism which is supposed to be
  // called on the fly as bytes are received
  if ( pfnWrite )
  {
    if ( static_cast<int>( pfnWrite( content.data(), 1, content.size(), pWriteArg ) ) != content.size() )
    {
      psResult->nStatus = 1;
      psResult->pszErrBuf = CPLStrdup( "download interrupted by user" );
      return psResult;
    }
  }
  else
  {
    psResult->nDataLen = static_cast<int>( content.size() );
    psResult->pabyData = static_cast<GByte *>( CPLMalloc( psResult->nDataLen + 1 ) );
    memcpy( psResult->pabyData, content.constData(), psResult->nDataLen );
    psResult->pabyData[psResult->nDataLen] = 0;
  }

  return psResult;
}

#endif

void QgsCPLHTTPFetchOverrider::setAttribute( QNetworkRequest::Attribute code, const QVariant &value )
{
  mAttributes[code] = value;
}
