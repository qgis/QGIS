/***************************************************************************
    qgsauthcerttrustpolicycombobox.cpp
    ---------------------
    begin                : May 02, 2015
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

#include "qgsauthcerttrustpolicycombobox.h"

#include <QLineEdit>

#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"


QgsAuthCertTrustPolicyComboBox::QgsAuthCertTrustPolicyComboBox( QWidget *parent,
    QgsAuthCertUtils::CertTrustPolicy policy,
    QgsAuthCertUtils::CertTrustPolicy defaultpolicy )
    : QComboBox( parent )
{
  QList < QPair<QgsAuthCertUtils::CertTrustPolicy, QString> > policies;
  policies << qMakePair( QgsAuthCertUtils::DefaultTrust,
                         defaultTrustText( defaultpolicy ) )
  << qMakePair( QgsAuthCertUtils::Trusted,
                QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Trusted ) )
  << qMakePair( QgsAuthCertUtils::Untrusted,
                QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::Untrusted ) );

  for ( int i = 0; i < policies.size(); i++ )
  {
    QgsAuthCertUtils::CertTrustPolicy polcy = policies.at( i ).first;
    QString name = policies.at( i ).second;
    addItem( name, QVariant(( int )polcy ) );
  }

  setItemData( 1, QgsAuthGuiUtils::greenColor(), Qt::TextColorRole );
  setItemData( 2, QgsAuthGuiUtils::redColor(), Qt::TextColorRole );

  // for styling closed state of combobox
//  setEditable( true );
//  lineEdit()->setReadOnly( true );

  connect( this, SIGNAL( currentIndexChanged( int ) ),
           this, SLOT( highlightCurrentIndex( int ) ) );

  setTrustPolicy( policy );
  setDefaultTrustPolicy( defaultpolicy );
}

QgsAuthCertTrustPolicyComboBox::~QgsAuthCertTrustPolicyComboBox()
{
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthCertTrustPolicyComboBox::trustPolicy()
{
  return ( QgsAuthCertUtils::CertTrustPolicy )itemData( currentIndex() ).toInt();
}

QgsAuthCertUtils::CertTrustPolicy QgsAuthCertTrustPolicyComboBox::trustPolicyForIndex( int indx )
{
  return ( QgsAuthCertUtils::CertTrustPolicy )itemData( indx ).toInt();
}

void QgsAuthCertTrustPolicyComboBox::setTrustPolicy( QgsAuthCertUtils::CertTrustPolicy policy )
{
  int idx = findData( QVariant(( int )policy ) );
  setCurrentIndex( idx == -1 ? 0 : idx );
}

void QgsAuthCertTrustPolicyComboBox::setDefaultTrustPolicy( QgsAuthCertUtils::CertTrustPolicy defaultpolicy )
{
  int idx = findData( QVariant(( int )QgsAuthCertUtils::DefaultTrust ) );
  setItemText( idx, defaultTrustText( defaultpolicy ) );
}

void QgsAuthCertTrustPolicyComboBox::highlightCurrentIndex( int indx )
{
  QgsAuthCertUtils::CertTrustPolicy policy = ( QgsAuthCertUtils::CertTrustPolicy )itemData( indx ).toInt();
  QString ss( "" );

  // TODO: why are these widget state selectors backwards?
  switch ( policy )
  {
    case QgsAuthCertUtils::Trusted:
      // ss = QgsAuthCertUtils::greenTextStyleSheet( "QLineEdit" );
      ss = QgsAuthGuiUtils::greenTextStyleSheet( "QComboBox:open" ) + "\nQComboBox:!open{}";
      break;
    case QgsAuthCertUtils::Untrusted:
      // ss = QgsAuthCertUtils::redTextStyleSheet( "QLineEdit" );
      ss = QgsAuthGuiUtils::redTextStyleSheet( "QComboBox:open" ) + "\nQComboBox:!open{}";
      break;
    case QgsAuthCertUtils::DefaultTrust:
    default:
      break;
  }
  QgsDebugMsg( QString( "Set Stylesheet to : %1" ).arg( ss ) );
  // lineEdit()->setStyleSheet( ss );
  setStyleSheet( ss );
}

const QString QgsAuthCertTrustPolicyComboBox::defaultTrustText( QgsAuthCertUtils::CertTrustPolicy defaultpolicy )
{
  if ( defaultpolicy == QgsAuthCertUtils::DefaultTrust )
  {
    if ( !QgsAuthManager::instance()->isDisabled() )
    {
      defaultpolicy = QgsAuthManager::instance()->defaultCertTrustPolicy();
    }
    else
    {
      defaultpolicy = QgsAuthCertUtils::Trusted;
    }
  }
  return QString( "%1 (%2)" )
         .arg( QgsAuthCertUtils::getCertTrustName( QgsAuthCertUtils::DefaultTrust ),
               QgsAuthCertUtils::getCertTrustName( defaultpolicy ) );
}
