/***************************************************************************
    qgsauthidentcertedit.cpp
    ---------------------
    begin                : September 1, 2015
    copyright            : (C) 2015 by Boundless Spatial, Inc. USA
    author               : Larry Shaffer
    email                : lshaffer at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsauthidentcertedit.h"
#include "ui_qgsauthidentcertedit.h"

#include "qgsapplication.h"
#include "qgsauthcertutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsAuthIdentCertEdit::QgsAuthIdentCertEdit( QWidget *parent )
    : QgsAuthMethodEdit( parent )
    , mValid( 0 )
{
  setupUi( this );
  populateIdentityComboBox();
}

QgsAuthIdentCertEdit::~QgsAuthIdentCertEdit()
{
}

bool QgsAuthIdentCertEdit::validateConfig()
{
  bool curvalid = cmbIdentityCert->currentIndex() != 0;
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

QgsStringMap QgsAuthIdentCertEdit::configMap() const
{
  QgsStringMap config;
  config.insert( "certid", cmbIdentityCert->itemData( cmbIdentityCert->currentIndex() ).toString() );

  return config;
}

void QgsAuthIdentCertEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  int indx = cmbIdentityCert->findData( configmap.value( "certid" ) );
  cmbIdentityCert->setCurrentIndex( indx == -1 ? 0 : indx );

  validateConfig();
}

void QgsAuthIdentCertEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthIdentCertEdit::clearConfig()
{
  cmbIdentityCert->setCurrentIndex( 0 );
}

void QgsAuthIdentCertEdit::populateIdentityComboBox()
{
  cmbIdentityCert->addItem( tr( "Select identity..." ), "" );

  QList<QSslCertificate> certs( QgsAuthManager::instance()->getCertIdentities() );
  if ( !certs.isEmpty() )
  {
    cmbIdentityCert->setIconSize( QSize( 26, 22 ) );
    QgsStringMap idents;
    Q_FOREACH ( const QSslCertificate& cert, certs )
    {
      QString org( SSL_SUBJECT_INFO( cert, QSslCertificate::Organization ) );
      if ( org.isEmpty() )
        org = tr( "Organization not defined" );
      idents.insert( QString( "%1 (%2)" ).arg( QgsAuthCertUtils::resolvedCertName( cert ), org ),
                     QgsAuthCertUtils::shaHexForCert( cert ) );
    }
    QgsStringMap::const_iterator it = idents.constBegin();
    for ( ; it != idents.constEnd(); ++it )
    {
      cmbIdentityCert->addItem( QgsApplication::getThemeIcon( "/mIconCertificate.svg" ),
                                it.key(), it.value() );
    }
  }
}

void QgsAuthIdentCertEdit::on_cmbIdentityCert_currentIndexChanged( int indx )
{
  Q_UNUSED( indx );
  validateConfig();
}
