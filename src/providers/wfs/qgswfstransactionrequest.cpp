/***************************************************************************
    qgswfstransactionrequest.cpp
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

#include "qgswfstransactionrequest.h"
#include "qgslogger.h"

QgsWFSTransactionRequest::QgsWFSTransactionRequest( const QString& theUri )
    : QgsWFSRequest( theUri )
{
}

bool QgsWFSTransactionRequest::send( const QDomDocument& doc, QDomDocument& serverResponse )
{
  QUrl url( baseURL() );

  QgsDebugMsg( doc.toString() );

  if ( sendPOST( url, "text/xml", doc.toByteArray( -1 ) ) )
  {
    QString errorMsg;
    if ( !serverResponse.setContent( mResponse, true, &errorMsg ) )
    {
      QgsDebugMsg( mResponse );
      QgsDebugMsg( errorMsg );
      return false;
    }
    return true;
  }
  return false;
}

QString QgsWFSTransactionRequest::errorMessageWithReason( const QString& reason )
{
  return tr( "Sending of transaction failed: %1" ).arg( reason );
}
