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

#include <QMessageBox>

void QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( QgsWfsCapabilities *capabilities, QWidget *parent )
{
  QString title;
  switch ( capabilities->errorCode() )
  {
    case QgsBaseNetworkRequest::NetworkError:
      title = QObject::tr( "Network Error" );
      break;
    case QgsBaseNetworkRequest::ServerExceptionError:
      title = QObject::tr( "Server Exception" );
      break;
    case QgsBaseNetworkRequest::ApplicationLevelError:
    {
      switch ( capabilities->applicationLevelError() )
      {
        case QgsWfsCapabilities::ApplicationLevelError::XmlError:
          title = QObject::tr( "Capabilities document is not valid" );
          break;
        case QgsWfsCapabilities::ApplicationLevelError::VersionNotSupported:
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
  QMessageBox *box = new QMessageBox( QMessageBox::Critical, title, capabilities->errorMessage(), QMessageBox::Ok, parent );
  box->setAttribute( Qt::WA_DeleteOnClose );
  box->setModal( true );
  box->setObjectName( QStringLiteral( "WFSCapabilitiesErrorBox" ) );
  if ( !parent->property( "hideDialogs" ).toBool() )
    box->open();
}
