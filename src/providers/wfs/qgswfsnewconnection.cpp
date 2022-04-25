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

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgswfsnewconnection.h"
#include "qgswfsguiutils.h"

#include <QMessageBox>

#include <algorithm>

QgsWFSNewConnection::QgsWFSNewConnection( QWidget *parent, const QString &connName ):
  QgsNewHttpConnection( parent, QgsNewHttpConnection::ConnectionWfs, QStringLiteral( "WFS" ), connName )
{
  connect( wfsVersionDetectButton(), &QPushButton::clicked, this, &QgsWFSNewConnection::versionDetectButton );
}

QgsWFSNewConnection::~QgsWFSNewConnection()
{
  if ( mCapabilities || mOAPIFLandingPage || mOAPIFApi )
  {
    QApplication::restoreOverrideCursor();
  }
}

QgsDataSourceUri QgsWFSNewConnection::createUri()
{
  // Honor any defined authentication settings
  QgsDataSourceUri uri;
  uri.setParam( QStringLiteral( "url" ), urlTrimmed().toString() );
  uri.setUsername( authSettingsWidget()->username() );
  uri.setPassword( authSettingsWidget()->password() );
  uri.setAuthConfigId( authSettingsWidget()->configId() );
  return uri;
}

void QgsWFSNewConnection::versionDetectButton()
{
  mCapabilities.reset( new QgsWfsCapabilities( createUri().uri( false ) ) );
  connect( mCapabilities.get(), &QgsWfsCapabilities::gotCapabilities, this, &QgsWFSNewConnection::capabilitiesReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  mCapabilities->setLogErrors( false ); // as this might be a OAPIF server
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

    mCapabilities.reset();
  }
}

void QgsWFSNewConnection::capabilitiesReplyFinished()
{
  if ( !mCapabilities )
    return;

  QApplication::restoreOverrideCursor();

  const auto err = mCapabilities->errorCode();
  if ( err != QgsBaseNetworkRequest::NoError )
  {
    startOapifLandingPageRequest();
    return;
  }

  const QgsWfsCapabilities::Capabilities &caps = mCapabilities->capabilities();
  int versionIdx = WFS_VERSION_MAX;
  wfsPageSizeLineEdit()->clear();
  if ( caps.version.startsWith( QLatin1String( "1.0" ) ) )
  {
    versionIdx = WFS_VERSION_1_0;
  }
  else if ( caps.version.startsWith( QLatin1String( "1.1" ) ) )
  {
    versionIdx = WFS_VERSION_1_1;
  }
  else if ( caps.version.startsWith( QLatin1String( "2.0" ) ) )
  {
    versionIdx = WFS_VERSION_2_0;
    wfsPageSizeLineEdit()->setText( QString::number( caps.maxFeatures ) );
  }
  wfsVersionComboBox()->setCurrentIndex( versionIdx );
  wfsPagingEnabledCheckBox()->setChecked( caps.supportsPaging );

  mCapabilities.reset();
}

void QgsWFSNewConnection::startOapifLandingPageRequest()
{
  mOAPIFLandingPage.reset( new QgsOapifLandingPageRequest( createUri() ) );
  connect( mOAPIFLandingPage.get(), &QgsOapifLandingPageRequest::gotResponse, this, &QgsWFSNewConnection::oapifLandingPageReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mOAPIFLandingPage->request( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get landing page" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFLandingPage.reset();
  }
}

void QgsWFSNewConnection::oapifLandingPageReplyFinished()
{
  if ( !mOAPIFLandingPage )
    return;

  QApplication::restoreOverrideCursor();

  if ( mOAPIFLandingPage->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    if ( mOAPIFLandingPage->errorCode() == QgsBaseNetworkRequest::ApplicationLevelError )
    {
      QMessageBox *box = new QMessageBox( QMessageBox::Critical, QObject::tr( "Invalid response" ), mOAPIFLandingPage->errorMessage(), QMessageBox::Ok, this );
      box->setAttribute( Qt::WA_DeleteOnClose );
      box->setModal( true );
      box->open();
    }
    else if ( mCapabilities )
    {
      QgsMessageLog::logMessage( mCapabilities->errorMessage(), tr( "WFS" ) );
      QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( mCapabilities.get(), this );
    }
    mCapabilities.reset();
    mOAPIFLandingPage.reset();
    return;
  }

  wfsVersionComboBox()->setCurrentIndex( WFS_VERSION_API_FEATURES_1_0 );
  wfsPagingEnabledCheckBox()->setChecked( true );

  mCapabilities.reset();

  startOapifApiRequest();
}

void QgsWFSNewConnection::startOapifApiRequest()
{
  Q_ASSERT( mOAPIFLandingPage );
  mOAPIFApi.reset( new QgsOapifApiRequest( createUri(), mOAPIFLandingPage->apiUrl() ) );
  mOAPIFLandingPage.reset();

  connect( mOAPIFApi.get(), &QgsOapifApiRequest::gotResponse, this, &QgsWFSNewConnection::oapifApiReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  if ( mOAPIFApi->request( synchronous, forceRefresh ) )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Could not get API" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFApi.reset();
  }
}

void QgsWFSNewConnection::oapifApiReplyFinished()
{
  if ( !mOAPIFApi )
    return;

  QApplication::restoreOverrideCursor();

  if ( mOAPIFApi->errorCode() != QgsBaseNetworkRequest::NoError )
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, QObject::tr( "Invalid response" ), mOAPIFApi->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFApi.reset();
    return;
  }

  wfsPageSizeLineEdit()->clear();
  if ( mOAPIFApi->defaultLimit() > 0 && mOAPIFApi->maxLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( std::min( std::max( 1000, mOAPIFApi->defaultLimit() ), mOAPIFApi->maxLimit() ) ) );
  else if ( mOAPIFApi->defaultLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( std::max( 1000, mOAPIFApi->defaultLimit() ) ) );
  else if ( mOAPIFApi->maxLimit() > 0 )
    wfsPageSizeLineEdit()->setText( QString::number( mOAPIFApi->maxLimit() ) );

  mOAPIFApi.reset();
}
