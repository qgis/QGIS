/***************************************************************************
    qgswfsfeaturehitsasyncrequest.cpp
    ---------------------------------
    begin                : January 2013
    copyright            : (C) 2013 by Marco Hugentobler
                           (C) 2016 by Even Rouault
    email                : marco dot hugentobler at sourcepole dot ch
                           even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsfeaturehitsasyncrequest.h"

#include "qgsfields.h"
#include "qgsgml.h"
#include "qgsmessagelog.h"

#include "moc_qgswfsfeaturehitsasyncrequest.cpp"

QgsWFSFeatureHitsAsyncRequest::QgsWFSFeatureHitsAsyncRequest( QgsWFSDataSourceURI &uri )
  : QgsWfsRequest( uri )
{
  connect( this, &QgsWfsRequest::downloadFinished, this, &QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished );
}

void QgsWFSFeatureHitsAsyncRequest::launchGet( const QUrl &url )
{
  sendGET( url,
           QString(), // content-type
           false,     /* synchronous */
           true,      /* forceRefresh */
           false /* cache */ );
}

void QgsWFSFeatureHitsAsyncRequest::launchPost( const QUrl &url, const QByteArray &data )
{
  sendPOST( url, u"application/xml; charset=utf-8"_s, data, false, /* synchronous */
            { QNetworkReply::RawHeaderPair { "Accept", "application/xml" } } );
}

void QgsWFSFeatureHitsAsyncRequest::hitsReplyFinished()
{
  if ( mErrorCode == NoError )
  {
    QByteArray data = response();
    QgsGmlStreamingParser gmlParser( ( QString() ), ( QString() ), QgsFields() );
    QString errorMsg;
    if ( gmlParser.processData( data, true, errorMsg ) )
    {
      mNumberMatched = ( gmlParser.numberMatched() >= 0 ) ? gmlParser.numberMatched() : gmlParser.numberReturned();
    }
    else
    {
      QgsMessageLog::logMessage( errorMsg, tr( "WFS" ) );
    }
  }
  emit gotHitsResponse();
}

QString QgsWFSFeatureHitsAsyncRequest::errorMessageWithReason( const QString &reason )
{
  return tr( "Download of feature count failed: %1" ).arg( reason );
}
