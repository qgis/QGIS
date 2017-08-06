/***************************************************************************
    qgsnewhttpconnection.cpp -  selector for a new HTTP server for WMS, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            : (C) 2005 by Brendan Morley
    email                : morb at ozemail dot com dot au
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsnewhttpconnection.h"
#include "qgsauthconfigselect.h"
#include "qgssettings.h"
#include "qgshelp.h"

#include <QMessageBox>
#include <QUrl>
#include <QPushButton>
#include <QRegExp>
#include <QRegExpValidator>

QgsNewHttpConnection::QgsNewHttpConnection(
  QWidget *parent, const QString &baseKey, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mBaseKey( baseKey )
  , mOriginalConnName( connName )
  , mAuthConfigSelect( nullptr )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewHttpConnection::showHelp );

  QRegExp rx( "/connections-([^/]+)/" );
  rx.indexIn( baseKey );
  setWindowTitle( tr( "Create a New %1 Connection" ).arg( rx.cap( 1 ).toUpper() ) );

  // It would be obviously much better to use mBaseKey also for credentials,
  // but for some strange reason a different hardcoded key was used instead.
  // WFS and WMS credentials were mixed with the same key WMS.
  // Only WMS and WFS providers are using QgsNewHttpConnection at this moment
  // using connection-wms and connection-wfs -> parse credential key fro it.
  mCredentialsBaseKey = mBaseKey.split( '-' ).last().toUpper();

  txtName->setValidator( new QRegExpValidator( QRegExp( "[^\\/]+" ), txtName ) );

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

  mAuthConfigSelect = new QgsAuthConfigSelect( this );
  tabAuth->insertTab( 1, mAuthConfigSelect, tr( "Configurations" ) );

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    QgsSettings settings;

    QString key = mBaseKey + connName;
    QString credentialsKey = "qgis/" + mCredentialsBaseKey + '/' + connName;
    txtName->setText( connName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );

    cbxIgnoreGetMapURI->setChecked( settings.value( key + "/ignoreGetMapURI", false ).toBool() );
    cbxIgnoreAxisOrientation->setChecked( settings.value( key + "/ignoreAxisOrientation", false ).toBool() );
    cbxInvertAxisOrientation->setChecked( settings.value( key + "/invertAxisOrientation", false ).toBool() );
    cbxIgnoreGetFeatureInfoURI->setChecked( settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool() );
    cbxSmoothPixmapTransform->setChecked( settings.value( key + "/smoothPixmapTransform", false ).toBool() );

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

  if ( mBaseKey != QLatin1String( "qgis/connections-wms/" ) )
  {
    if ( mBaseKey != QLatin1String( "qgis/connections-wcs/" ) &&
         mBaseKey != QLatin1String( "qgis/connections-wfs/" ) )
    {
      cbxIgnoreAxisOrientation->setVisible( false );
      cbxInvertAxisOrientation->setVisible( false );
      mGroupBox->layout()->removeWidget( cbxIgnoreAxisOrientation );
      mGroupBox->layout()->removeWidget( cbxInvertAxisOrientation );
    }

    if ( mBaseKey == QLatin1String( "qgis/connections-wfs/" ) )
    {
      cbxIgnoreAxisOrientation->setText( tr( "Ignore axis orientation (WFS 1.1/WFS 2.0)" ) );
    }

    if ( mBaseKey == QLatin1String( "qgis/connections-wcs/" ) )
    {
      cbxIgnoreGetMapURI->setText( tr( "Ignore GetCoverage URI reported in capabilities" ) );
      cbxIgnoreAxisOrientation->setText( tr( "Ignore axis orientation" ) );
    }
    else
    {
      cbxIgnoreGetMapURI->setVisible( false );
      cbxSmoothPixmapTransform->setVisible( false );
      mGroupBox->layout()->removeWidget( cbxIgnoreGetMapURI );
      mGroupBox->layout()->removeWidget( cbxSmoothPixmapTransform );
    }

    cbxIgnoreGetFeatureInfoURI->setVisible( false );
    mGroupBox->layout()->removeWidget( cbxIgnoreGetFeatureInfoURI );

    cmbDpiMode->setVisible( false );
    mGroupBox->layout()->removeWidget( cmbDpiMode );
    lblDpiMode->setVisible( false );
    mGroupBox->layout()->removeWidget( lblDpiMode );

    txtReferer->setVisible( false );
    mGroupBox->layout()->removeWidget( txtReferer );
    lblReferer->setVisible( false );
    mGroupBox->layout()->removeWidget( lblReferer );
  }

  if ( mBaseKey != QLatin1String( "qgis/connections-wfs/" ) )
  {
    lblVersion->setVisible( false );
    cmbVersion->setVisible( false );
    mGroupBox->layout()->removeWidget( cmbVersion );
    lblMaxNumFeatures->setVisible( false );
    mGroupBox->layout()->removeWidget( lblMaxNumFeatures );
    txtMaxNumFeatures->setVisible( false );
    mGroupBox->layout()->removeWidget( txtMaxNumFeatures );
  }

  // Adjust height
  int w = width();
  adjustSize();
  resize( w, height() );

  on_txtName_textChanged( connName );
}

void QgsNewHttpConnection::on_txtName_textChanged( const QString &text )
{
  Q_UNUSED( text );
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
}

void QgsNewHttpConnection::on_txtUrl_textChanged( const QString &text )
{
  Q_UNUSED( text );
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( txtName->text().isEmpty() || txtUrl->text().isEmpty() );
}

void QgsNewHttpConnection::accept()
{
  QgsSettings settings;
  QString key = mBaseKey + txtName->text();
  QString credentialsKey = "qgis/" + mCredentialsBaseKey + '/' + txtName->text();

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
    settings.remove( mBaseKey + mOriginalConnName );
    settings.remove( "qgis/" + mCredentialsBaseKey + '/' + mOriginalConnName );
    settings.sync();
  }

  QUrl url( txtUrl->text().trimmed() );
  const QList< QPair<QByteArray, QByteArray> > &items = url.encodedQueryItems();
  QHash< QString, QPair<QByteArray, QByteArray> > params;
  for ( QList< QPair<QByteArray, QByteArray> >::const_iterator it = items.constBegin(); it != items.constEnd(); ++it )
  {
    params.insert( QString( it->first ).toUpper(), *it );
  }

  if ( params[QStringLiteral( "SERVICE" )].second.toUpper() == "WMS" ||
       params[QStringLiteral( "SERVICE" )].second.toUpper() == "WFS" ||
       params[QStringLiteral( "SERVICE" )].second.toUpper() == "WCS" )
  {
    url.removeEncodedQueryItem( params[QStringLiteral( "SERVICE" )].first );
    url.removeEncodedQueryItem( params[QStringLiteral( "REQUEST" )].first );
    url.removeEncodedQueryItem( params[QStringLiteral( "FORMAT" )].first );
  }

  if ( url.encodedPath().isEmpty() )
  {
    url.setEncodedPath( "/" );
  }

  settings.setValue( key + "/url", url.toString() );

  if ( mBaseKey == QLatin1String( "qgis/connections-wms/" ) ||
       mBaseKey == QLatin1String( "qgis/connections-wcs/" ) ||
       mBaseKey == QLatin1String( "qgis/connections-wfs/" ) )
  {
    settings.setValue( key + "/ignoreAxisOrientation", cbxIgnoreAxisOrientation->isChecked() );
    settings.setValue( key + "/invertAxisOrientation", cbxInvertAxisOrientation->isChecked() );
  }

  if ( mBaseKey == QLatin1String( "qgis/connections-wms/" ) || mBaseKey == QLatin1String( "qgis/connections-wcs/" ) )
  {
    settings.setValue( key + "/ignoreGetMapURI", cbxIgnoreGetMapURI->isChecked() );
    settings.setValue( key + "/smoothPixmapTransform", cbxSmoothPixmapTransform->isChecked() );

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

    settings.setValue( key + "/dpiMode", dpiMode );
  }
  if ( mBaseKey == QLatin1String( "qgis/connections-wms/" ) )
  {
    settings.setValue( key + "/ignoreGetFeatureInfoURI", cbxIgnoreGetFeatureInfoURI->isChecked() );
  }
  if ( mBaseKey == QLatin1String( "qgis/connections-wfs/" ) )
  {
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
    settings.setValue( key + "/version", version );

    settings.setValue( key + "/maxnumfeatures", txtMaxNumFeatures->text() );
  }

  settings.setValue( key + "/referer", txtReferer->text() );

  settings.setValue( credentialsKey + "/username", txtUserName->text() );
  settings.setValue( credentialsKey + "/password", txtPassword->text() );

  settings.setValue( credentialsKey + "/authcfg", mAuthConfigSelect->configId() );

  settings.setValue( mBaseKey + "/selected", txtName->text() );

  QDialog::accept();
}

void QgsNewHttpConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/index.html" ) );
}
