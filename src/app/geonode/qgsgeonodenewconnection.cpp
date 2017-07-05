/***************************************************************************
                              qgsgeonodenewconnection.cpp
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QMessageBox>
#include <QInputDialog>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "qgsgeonodenewconnection.h"
#include "qgsauthmanager.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgsgeonodeconnection.h"
#include "qgssettings.h"
#include "qgsnetworkaccessmanager.h"

QgsGeoNodeNewConnection::QgsGeoNodeNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
  , mAuthConfigSelect( nullptr )
{
  setupUi( this );

  mBaseKey = QgsGeoNodeConnection::pathGeoNodeConnection;
  mCredentialsBaseKey = QgsGeoNodeConnection::pathGeoNodeConnectionDetails;

  mAuthConfigSelect = new QgsAuthConfigSelect( this );
  tabAuth->insertTab( 1, mAuthConfigSelect, tr( "Configurations" ) );

  cmbDpiMode->clear();
  cmbDpiMode->addItem( tr( "all" ) );
  cmbDpiMode->addItem( tr( "off" ) );
  cmbDpiMode->addItem( tr( "QGIS" ) );
  cmbDpiMode->addItem( tr( "UMN" ) );
  cmbDpiMode->addItem( tr( "GeoServer" ) );

  cmbVersion->clear();
  cmbVersion->addItem( tr( "Auto-detect" ) );
  cmbVersion->addItem( tr( "1.0" ) );
  cmbVersion->addItem( tr( "1.1" ) );
  cmbVersion->addItem( tr( "2.0" ) );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = mBaseKey + '/' + connName;
    QString credentialsKey = mCredentialsBaseKey + '/' + connName;
    txtName->setText( connName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );

    cbxIgnoreGetMapURI->setChecked( settings.value( key + "/wms/ignoreGetMapURI", false ).toBool() );
    cbxWfsIgnoreAxisOrientation->setChecked( settings.value( key + "/wfs/ignoreAxisOrientation", false ).toBool() );
    cbxWmsIgnoreAxisOrientation->setChecked( settings.value( key + "/wms/ignoreAxisOrientation", false ).toBool() );
    cbxWfsInvertAxisOrientation->setChecked( settings.value( key + "/wfs/invertAxisOrientation", false ).toBool() );
    cbxWmsInvertAxisOrientation->setChecked( settings.value( key + "/wms/invertAxisOrientation", false ).toBool() );
    cbxIgnoreGetFeatureInfoURI->setChecked( settings.value( key + "/wms/ignoreGetFeatureInfoURI", false ).toBool() );
    cbxSmoothPixmapTransform->setChecked( settings.value( key + "/wms/smoothPixmapTransform", false ).toBool() );

    int dpiIdx;
    switch ( settings.value( key + "/dpiMode", 7 ).toInt() )
    {
      case 0: // off
        dpiIdx = 1;
        break;
      case 1: // QGIS
        dpiIdx = 2;
        break;
      case 2: // UMN
        dpiIdx = 3;
        break;
      case 4: // GeoServer
        dpiIdx = 4;
        break;
      default: // other => all
        dpiIdx = 0;
        break;
    }
    cmbDpiMode->setCurrentIndex( dpiIdx );

    QString version = settings.value( key + "/version" ).toString();
    int versionIdx = 0; // AUTO
    if ( version == QLatin1String( "1.0.0" ) )
      versionIdx = 1;
    else if ( version == QLatin1String( "1.1.0" ) )
      versionIdx = 2;
    else if ( version == QLatin1String( "2.0.0" ) )
      versionIdx = 3;
    cmbVersion->setCurrentIndex( versionIdx );

    txtReferer->setText( settings.value( key + "/referer" ).toString() );
    txtMaxNumFeatures->setText( settings.value( key + "/maxnumfeatures" ).toString() );

    txtUserName->setText( settings.value( credentialsKey + "/username" ).toString() );
    txtPassword->setText( settings.value( credentialsKey + "/password" ).toString() );

    QString authcfg = settings.value( credentialsKey + "/authcfg" ).toString();
    mAuthConfigSelect->setConfigId( authcfg );
    if ( !authcfg.isEmpty() )
    {
      tabAuth->setCurrentIndex( tabAuth->indexOf( mAuthConfigSelect ) );
    }
  }

  // Adjust height
  int w = width();
  adjustSize();
  resize( w, height() );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsGeoNodeNewConnection::okButtonBehavior );
  connect( txtUrl, &QLineEdit::textChanged, this, &QgsGeoNodeNewConnection::okButtonBehavior );
  connect( btnConnect, &QPushButton::clicked, this, &QgsGeoNodeNewConnection::testConnection );
}

void QgsGeoNodeNewConnection::accept()
{
  QgsSettings settings;
  QString key = mBaseKey + '/' + txtName->text();
  QString credentialsKey = mCredentialsBaseKey + '/' + txtName->text();

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       settings.contains( key + "/url" ) &&
       QMessageBox::question( this,
                              tr( "Save connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  if ( !txtPassword->text().isEmpty() &&
       QMessageBox::question( this,
                              tr( "Saving passwords" ),
                              trUtf8( "WARNING: You have entered a password. It will be stored in unsecured plain text in your project files and your home directory (Unix-like OS) or user profile (Windows). If you want to avoid this, press Cancel and either:\n\na) Don't provide a password in the connection settings — it will be requested interactively when needed;\nb) Use the Configuration tab to add your credentials in an HTTP Basic Authentication method and store them in an encrypted database." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != key )
  {
    settings.remove( mBaseKey + '/' + mOriginalConnName );
    settings.remove( "qgis//" + mCredentialsBaseKey + '/' + mOriginalConnName );
    settings.sync();
  }

  if ( !txtUrl->text().contains( "://" ) &&
       QMessageBox::information(
         this,
         tr( "Invalid URL" ),
         tr( "Your URL doesn't contains protocol (e.g. http or https). Please add the protocol." ) ) == QMessageBox::Ok )
  {
    return;
  }
  QUrl url( txtUrl->text() );

  settings.setValue( key + "/url", url.toString() );

  settings.setValue( key + "/wfs/ignoreAxisOrientation", cbxWfsIgnoreAxisOrientation->isChecked() );
  settings.setValue( key + "/wms/ignoreAxisOrientation", cbxWmsIgnoreAxisOrientation->isChecked() );
  settings.setValue( key + "/wfs/invertAxisOrientation", cbxWfsInvertAxisOrientation->isChecked() );
  settings.setValue( key + "/wms/invertAxisOrientation", cbxWmsInvertAxisOrientation->isChecked() );

  settings.setValue( key + "/wms/ignoreGetMapURI", cbxIgnoreGetMapURI->isChecked() );
  settings.setValue( key + "/wms/smoothPixmapTransform", cbxSmoothPixmapTransform->isChecked() );
  settings.setValue( key + "/wms/ignoreGetFeatureInfoURI", cbxIgnoreGetFeatureInfoURI->isChecked() );

  int dpiMode = 0;
  switch ( cmbDpiMode->currentIndex() )
  {
    case 0: // all => QGIS|UMN|GeoServer
      dpiMode = 7;
      break;
    case 1: // off
      dpiMode = 0;
      break;
    case 2: // QGIS
      dpiMode = 1;
      break;
    case 3: // UMN
      dpiMode = 2;
      break;
    case 4: // GeoServer
      dpiMode = 4;
      break;
  }

  settings.setValue( key + "/wms/dpiMode", dpiMode );
  settings.setValue( key + "/wms/referer", txtReferer->text() );

  QString version = QStringLiteral( "auto" );
  switch ( cmbVersion->currentIndex() )
  {
    case 0:
      version = QStringLiteral( "auto" );
      break;
    case 1:
      version = QStringLiteral( "1.0.0" );
      break;
    case 2:
      version = QStringLiteral( "1.1.0" );
      break;
    case 3:
      version = QStringLiteral( "2.0.0" );
      break;
  }

  settings.setValue( key + "/wfs/version", version );
  settings.setValue( key + "/wfs/maxnumfeatures", txtMaxNumFeatures->text() );

  settings.setValue( credentialsKey + "/username", txtUserName->text() );
  settings.setValue( credentialsKey + "/password", txtPassword->text() );

  settings.setValue( credentialsKey + "/authcfg", mAuthConfigSelect->configId() );

  settings.setValue( mBaseKey + "/selected", txtName->text() );

  QDialog::accept();
}

void QgsGeoNodeNewConnection::okButtonBehavior( const QString &text )
{
  Q_UNUSED( text );
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( !txtName->text().isEmpty() && !txtUrl->text().isEmpty() );
}

void QgsGeoNodeNewConnection::testConnection()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  QString endpoint( "/api/layers/" );
  QNetworkReply *layersReply = request( endpoint );
  endpoint = "/api/maps";
  QNetworkReply *mapsReply = request( endpoint );
  QApplication::restoreOverrideCursor();

  if ( layersReply->error() == QNetworkReply::NoError && mapsReply->error() == QNetworkReply::NoError )
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "\nConnection to %1 was successful, \n\n%1 is a valid geonode instance.\n\n" ).arg( txtUrl->text() ) );
  }
  else
  {
    QMessageBox::information( this,
                              tr( "Test connection" ),
                              tr( "\nConnection failed, \n\nplease check whether %1 is a valid geonode instance.\n\n" ).arg( txtUrl->text() ) );
  }
}

QNetworkReply *QgsGeoNodeNewConnection::request( QString &endPoint )
{
  QString url = txtUrl->text() + endPoint;
  if ( !url.contains( QLatin1String( "://" ) ) )
  {
    url.prepend( "http://" );
  }

  QUrl layerUrl( url );
  layerUrl.setScheme( "http" );
  QgsNetworkAccessManager *networkManager = QgsNetworkAccessManager::instance();

  QNetworkRequest request( layerUrl );
  request.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );

  QNetworkReply *reply = networkManager->get( request );
  while ( !reply->isFinished() )
  {
    qApp->processEvents();
  }

  return reply;
}
