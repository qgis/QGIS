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
#include "qgscontexthelp.h"
#include <QSettings>
#include <QMessageBox>
#include <QUrl>
#include <QPushButton>

QgsNewHttpConnection::QgsNewHttpConnection(
  QWidget *parent, const QString& baseKey, const QString& connName, Qt::WFlags fl ):
    QDialog( parent, fl ),
    mBaseKey( baseKey ),
    mOriginalConnName( connName )
{
  setupUi( this );

  QString service = baseKey.mid( 18, 3 ).toUpper();
  setWindowTitle( tr( "Create a new %1 connection" ).arg( service ) );

  // It would be obviously much better to use mBaseKey also for credentials,
  // but for some strange reason a different hardcoded key was used instead.
  // WFS and WMS credentials were mixed with the same key WMS.
  // Only WMS and WFS providers are using QgsNewHttpConnection at this moment
  // using connection-wms and connection-wfs -> parse credential key fro it.
  mCredentialsBaseKey = mBaseKey.split( '-' ).last().toUpper();

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters

    QSettings settings;

    QString key = mBaseKey + connName;
    QString credentialsKey = "/Qgis/" + mCredentialsBaseKey + "/" + connName;
    txtName->setText( connName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );

    cbxIgnoreGetMapURI->setChecked( settings.value( key + "/ignoreGetMapURI", false ).toBool() );
    cbxIgnoreAxisOrientation->setChecked( settings.value( key + "/ignoreAxisOrientation", false ).toBool() );
    cbxInvertAxisOrientation->setChecked( settings.value( key + "/invertAxisOrientation", false ).toBool() );
    cbxIgnoreGetFeatureInfoURI->setChecked( settings.value( key + "/ignoreGetFeatureInfoURI", false ).toBool() );
    cbxSmoothPixmapTransform->setChecked( settings.value( key + "/smoothPixmapTransform", false ).toBool() );

    txtReferer->setText( settings.value( key + "/referer" ).toString() );

    txtUserName->setText( settings.value( credentialsKey + "/username" ).toString() );
    txtPassword->setText( settings.value( credentialsKey + "/password" ).toString() );
  }

  if ( mBaseKey != "/Qgis/connections-wms/" )
  {
    if ( mBaseKey == "/Qgis/connections-wcs/" )
    {
      cbxIgnoreGetMapURI->setText( tr( "Ignore GetCoverage URI reported in capabilities" ) );
      cbxIgnoreAxisOrientation->setText( tr( "Ignore axis orientation" ) );
    }
    else
    {
      cbxIgnoreGetMapURI->setVisible( false );
      cbxIgnoreAxisOrientation->setVisible( false );
      cbxInvertAxisOrientation->setVisible( false );
      cbxSmoothPixmapTransform->setVisible( false );
      mGroupBox->layout()->removeWidget( cbxIgnoreGetMapURI );
      mGroupBox->layout()->removeWidget( cbxIgnoreAxisOrientation );
      mGroupBox->layout()->removeWidget( cbxInvertAxisOrientation );
      mGroupBox->layout()->removeWidget( cbxSmoothPixmapTransform );
    }

    cbxIgnoreGetFeatureInfoURI->setVisible( false );
    mGroupBox->layout()->removeWidget( cbxIgnoreGetFeatureInfoURI );

    txtReferer->setVisible( false );
    mGroupBox->layout()->removeWidget( txtReferer );
    lblReferer->setVisible( false );
    mGroupBox->layout()->removeWidget( lblReferer );

    // Adjust height
    int w = width();
    adjustSize();
    resize( w, height() );
  }

  on_txtName_textChanged( connName );
}

QgsNewHttpConnection::~QgsNewHttpConnection()
{
}

void QgsNewHttpConnection::on_txtName_textChanged( const QString &text )
{
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( text.isEmpty() );
}

void QgsNewHttpConnection::accept()
{
  QSettings settings;
  QString key = mBaseKey + txtName->text();
  QString credentialsKey = "/Qgis/" + mCredentialsBaseKey + "/" + txtName->text();

  // warn if entry was renamed to an existing connection
  if (( mOriginalConnName.isNull() || mOriginalConnName != txtName->text() ) &&
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
                              tr( "WARNING: You have entered a password. It will be stored in plain text in your project files and in your home directory on Unix-like systems, or in your user profile on Windows. If you do not want this to happen, please press the Cancel button.\nNote: giving the password is optional. It will be requested interactivly, when needed." ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != key )
  {
    settings.remove( mBaseKey + mOriginalConnName );
    settings.remove( "/Qgis/" + mCredentialsBaseKey + "/" + mOriginalConnName );
  }

  QUrl url( txtUrl->text().trimmed() );
  const QList< QPair<QByteArray, QByteArray> > &items = url.encodedQueryItems();
  QHash< QString, QPair<QByteArray, QByteArray> > params;
  for ( QList< QPair<QByteArray, QByteArray> >::const_iterator it = items.constBegin(); it != items.constEnd(); ++it )
  {
    params.insert( QString( it->first ).toUpper(), *it );
  }

  if ( params["SERVICE"].second.toUpper() == "WMS" ||
       params["SERVICE"].second.toUpper() == "WFS" ||
       params["SERVICE"].second.toUpper() == "WCS" )
  {
    url.removeEncodedQueryItem( params["SERVICE"].first );
    url.removeEncodedQueryItem( params["REQUEST"].first );
    url.removeEncodedQueryItem( params["FORMAT"].first );
  }

  if ( url.encodedPath().isEmpty() )
  {
    url.setEncodedPath( "/" );
  }

  settings.setValue( key + "/url", url.toString() );
  if ( mBaseKey == "/Qgis/connections-wms/" || mBaseKey == "/Qgis/connections-wcs/" )
  {
    settings.setValue( key + "/ignoreGetMapURI", cbxIgnoreGetMapURI->isChecked() );
    settings.setValue( key + "/ignoreAxisOrientation", cbxIgnoreAxisOrientation->isChecked() );
    settings.setValue( key + "/invertAxisOrientation", cbxInvertAxisOrientation->isChecked() );
    settings.setValue( key + "/smoothPixmapTransform", cbxSmoothPixmapTransform->isChecked() );
  }
  if ( mBaseKey == "/Qgis/connections-wms/" )
  {
    settings.setValue( key + "/ignoreGetFeatureInfoURI", cbxIgnoreGetFeatureInfoURI->isChecked() );
  }

  settings.setValue( key + "/referer", txtReferer->text() );

  settings.setValue( credentialsKey + "/username", txtUserName->text() );
  settings.setValue( credentialsKey + "/password", txtPassword->text() );

  settings.setValue( mBaseKey + "/selected", txtName->text() );

  QDialog::accept();
}
