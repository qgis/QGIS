/***************************************************************************
    qgswfsguiutils.h
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

#include "qgswfsguiutils.h"

#include "qgswfsgetcapabilities.h"

#include <QMessageBox>

void QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( QgsWfsGetCapabilitiesRequest *request, QWidget *parent )
{
  QString title;
  switch ( request->errorCode() )
  {
    case QgsBaseNetworkRequest::NetworkError:
      title = QObject::tr( "Network Error" );
      break;
    case QgsBaseNetworkRequest::ServerExceptionError:
      title = QObject::tr( "Server Exception" );
      break;
    case QgsBaseNetworkRequest::ApplicationLevelError:
    {
      switch ( request->applicationLevelError() )
      {
        case QgsWfsGetCapabilitiesRequest::ApplicationLevelError::XmlError:
          title = QObject::tr( "Capabilities document is not valid" );
          break;
        case QgsWfsGetCapabilitiesRequest::ApplicationLevelError::VersionNotSupported:
          title = QObject::tr( "WFS version not supported" );
          break;
        default:
          title = QObject::tr( "Error" );
          break;
      }
      break;
    }
    default:
      title = QObject::tr( "Error" );
      break;
  }
  // handle errors
  QMessageBox *box = new QMessageBox( QMessageBox::Critical, title, request->errorMessage(), QMessageBox::Ok, parent );
  box->setAttribute( Qt::WA_DeleteOnClose );
  box->setModal( true );
  box->setObjectName( u"WFSCapabilitiesErrorBox"_s );
  if ( !parent->property( "hideDialogs" ).toBool() )
    box->open();
}
