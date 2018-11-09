/***************************************************************************
    qgswfsnewconnection.cpp
    ---------------------
    begin                : June 2018
    copyright            : (C) 2018 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswfsnewconnection.h"

#include <QMessageBox>

QgsWFSNewConnection::QgsWFSNewConnection( QWidget *parent, const QString &connName ):
  QgsNewHttpConnection( parent, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS, connName )
{
  connect( wfsVersionDetectButton(), &QPushButton::clicked, this, &QgsWFSNewConnection::versionDetectButton );
}

QgsWFSNewConnection::~QgsWFSNewConnection()
{
  if ( mCapabilities )
  {
    QApplication::restoreOverrideCursor();
    delete mCapabilities;
  }
}

void QgsWFSNewConnection::versionDetectButton()
{
  delete mCapabilities;
  mCapabilities = new QgsWfsCapabilities( urlTrimmed().toString() );
  connect( mCapabilities, &QgsWfsCapabilities::gotCapabilities, this, &QgsWFSNewConnection::capabilitiesReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mCapabilities->requestCapabilities( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get capabilities" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    delete mCapabilities;
    mCapabilities = nullptr;
  }
}

void QgsWFSNewConnection::capabilitiesReplyFinished()
{
  if ( !mCapabilities )
    return;

  QApplication::restoreOverrideCursor();

  QgsWfsCapabilities::ErrorCode err = mCapabilities->errorCode();
  if ( err != QgsWfsCapabilities::NoError )
  {
    QString title;
    switch ( err )
    {
      case QgsWfsCapabilities::NetworkError:
        title = tr( "Network Error" );
        break;
      case QgsWfsCapabilities::XmlError:
        title = tr( "Capabilities document is not valid" );
        break;
      case QgsWfsCapabilities::ServerExceptionError:
        title = tr( "Server Exception" );
        break;
      default:
        title = tr( "Error" );
        break;
    }
    // handle errors
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, title, mCapabilities->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    delete mCapabilities;
    mCapabilities = nullptr;
    return;
  }

  const auto &caps = mCapabilities->capabilities();
  int versionIdx = 0;
  wfsPageSizeLineEdit()->clear();
  if ( caps.version.startsWith( QLatin1String( "1.0" ) ) )
  {
    versionIdx = 1;
  }
  else if ( caps.version.startsWith( QLatin1String( "1.1" ) ) )
  {
    versionIdx = 2;
  }
  else if ( caps.version.startsWith( QLatin1String( "2.0" ) ) )
  {
    versionIdx = 3;
    wfsPageSizeLineEdit()->setText( QString::number( caps.maxFeatures ) );
  }
  wfsVersionComboBox()->setCurrentIndex( versionIdx );
  wfsPagingEnabledCheckBox()->setChecked( caps.supportsPaging );

  delete mCapabilities;
  mCapabilities = nullptr;
}
