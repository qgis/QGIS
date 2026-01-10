/***************************************************************************
    qgsauthpkipathsedit.cpp
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

#include "ui_qgsauthpkipathsedit.h"
#include "qgsauthpkipathsedit.h"

#include "qgsapplication.h"
#include "qgsauthcertutils.h"
#include "qgsauthguiutils.h"
#include "qgsauthmanager.h"
#include "qgslogger.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QSslCertificate>
#include <QSslKey>

#include "moc_qgsauthpkipathsedit.cpp"

QgsAuthPkiPathsEdit::QgsAuthPkiPathsEdit( QWidget *parent )
  : QgsAuthMethodEdit( parent )
{
  setupUi( this );
  connect( btnPkiPathsCert, &QToolButton::clicked, this, &QgsAuthPkiPathsEdit::btnPkiPathsCert_clicked );
  connect( btnPkiPathsKey, &QToolButton::clicked, this, &QgsAuthPkiPathsEdit::btnPkiPathsKey_clicked );
  connect( cbAddCas, &QCheckBox::stateChanged, this, [this]( int state ) { cbAddRootCa->setEnabled( state == Qt::Checked ); } );
  lblCas->hide();
  twCas->hide();
  cbAddCas->hide();
  cbAddRootCa->hide();
}

bool QgsAuthPkiPathsEdit::validateConfig()
{
  // required components
  const QString certpath( lePkiPathsCert->text() );
  const QString keypath( lePkiPathsKey->text() );

  const bool certfound = QFile::exists( certpath );
  const bool keyfound = QFile::exists( keypath );

  QgsAuthGuiUtils::fileFound( certpath.isEmpty() || certfound, lePkiPathsCert );
  QgsAuthGuiUtils::fileFound( keypath.isEmpty() || keyfound, lePkiPathsKey );

  if ( !certfound || !keyfound )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Missing components" ), Invalid );
    return validityChange( false );
  }

  // check for issue date validity, then notify status
  const QSslCertificate cert( QgsAuthCertUtils::certFromFile( certpath ) );

  if ( cert.isNull() )
  {
    writePkiMessage( lePkiPathsMsg, tr( "Failed to load certificate from file" ), Invalid );
    return validityChange( false );
  }

  const QDateTime startdate( cert.effectiveDate() );
  const QDateTime enddate( cert.expiryDate() );

  writePkiMessage( lePkiPathsMsg, tr( "%1 thru %2" ).arg( startdate.toString(), enddate.toString() ), ( QgsAuthCertUtils::certIsCurrent( cert ) ? Valid : Invalid ) );

  const bool certviable = QgsAuthCertUtils::certIsViable( cert );
  const bool showCas( certviable && populateCas() );
  lblCas->setVisible( showCas );
  twCas->setVisible( showCas );
  cbAddCas->setVisible( showCas );
  cbAddRootCa->setVisible( showCas );

  return validityChange( certviable );
}

QgsStringMap QgsAuthPkiPathsEdit::configMap() const
{
  QgsStringMap config;
  config.insert( u"certpath"_s, lePkiPathsCert->text() );
  config.insert( u"keypath"_s, lePkiPathsKey->text() );
  config.insert( u"keypass"_s, lePkiPathsKeyPass->text() );
  config.insert( u"addcas"_s, cbAddCas->isChecked() ? u"true"_s : u"false"_s );
  config.insert( u"addrootca"_s, cbAddRootCa->isChecked() ? u"true"_s : u"false"_s );

  return config;
}

void QgsAuthPkiPathsEdit::loadConfig( const QgsStringMap &configmap )
{
  clearConfig();

  mConfigMap = configmap;
  lePkiPathsCert->setText( configmap.value( u"certpath"_s ) );
  lePkiPathsKey->setText( configmap.value( u"keypath"_s ) );
  lePkiPathsKeyPass->setText( configmap.value( u"keypass"_s ) );
  cbAddCas->setChecked( configmap.value( u"addcas"_s, u"false "_s ) == "true"_L1 );
  cbAddRootCa->setChecked( configmap.value( u"addrootca"_s, u"false "_s ) == "true"_L1 );

  validateConfig();
}

void QgsAuthPkiPathsEdit::resetConfig()
{
  loadConfig( mConfigMap );
}

void QgsAuthPkiPathsEdit::clearConfig()
{
  clearPkiPathsCertPath();
  clearPkiPathsKeyPath();
  clearPkiPathsKeyPass();

  clearPkiMessage( lePkiPathsMsg );
  validateConfig();
}

void QgsAuthPkiPathsEdit::clearPkiMessage( QLineEdit *lineedit )
{
  lineedit->clear();
  lineedit->setStyleSheet( QString() );
}

void QgsAuthPkiPathsEdit::writePkiMessage( QLineEdit *lineedit, const QString &msg, Validity valid )
{
  QString ss;
  QString txt( msg );
  switch ( valid )
  {
    case Valid:
      ss = QgsAuthGuiUtils::greenTextStyleSheet( u"QLineEdit"_s );
      txt = tr( "Valid: %1" ).arg( msg );
      break;
    case Invalid:
      ss = QgsAuthGuiUtils::redTextStyleSheet( u"QLineEdit"_s );
      txt = tr( "Invalid: %1" ).arg( msg );
      break;
    case Unknown:
      break;
  }
  lineedit->setStyleSheet( ss );
  lineedit->setText( txt );
  lineedit->setCursorPosition( 0 );
}

void QgsAuthPkiPathsEdit::clearPkiPathsCertPath()
{
  lePkiPathsCert->clear();
  lePkiPathsCert->setStyleSheet( QString() );
}

void QgsAuthPkiPathsEdit::clearPkiPathsKeyPath()
{
  lePkiPathsKey->clear();
  lePkiPathsKey->setStyleSheet( QString() );
}

void QgsAuthPkiPathsEdit::clearPkiPathsKeyPass()
{
  lePkiPathsKeyPass->clear();
  lePkiPathsKeyPass->setStyleSheet( QString() );
}

void QgsAuthPkiPathsEdit::btnPkiPathsCert_clicked()
{
  const QString &fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open Client Certificate File" ), tr( "All files (*.*);;PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsCert->setText( fn );
    validateConfig();
  }
}

void QgsAuthPkiPathsEdit::btnPkiPathsKey_clicked()
{
  const QString &fn = QgsAuthGuiUtils::getOpenFileName( this, tr( "Open Private Key File" ), tr( "All files (*.*);;PEM (*.pem);;DER (*.der)" ) );
  if ( !fn.isEmpty() )
  {
    lePkiPathsKey->setText( fn );
    validateConfig();
  }
}

bool QgsAuthPkiPathsEdit::validityChange( bool curvalid )
{
  if ( mValid != curvalid )
  {
    mValid = curvalid;
    emit validityChanged( curvalid );
  }
  return curvalid;
}

bool QgsAuthPkiPathsEdit::populateCas()
{
  twCas->clear();
  const QList<QSslCertificate> cas( QgsAuthCertUtils::casFromFile( lePkiPathsCert->text() ) );
  if ( cas.isEmpty() )
  {
    return false;
  }

  QTreeWidgetItem *prevItem( nullptr );
  QList<QSslCertificate>::const_iterator it( cas.constEnd() );
  while ( it != cas.constBegin() )
  {
    --it;
    const QSslCertificate cert = static_cast<QSslCertificate>( *it );
    QTreeWidgetItem *item;

    if ( prevItem && cert.issuerInfo( QSslCertificate::SubjectInfo::CommonName ).contains( prevItem->text( 0 ) ) )
    {
      item = new QTreeWidgetItem( QStringList( cert.subjectInfo( QSslCertificate::SubjectInfo::CommonName ) ) );
      prevItem->addChild( item );
    }
    else
    {
      item = new QTreeWidgetItem( twCas, QStringList( cert.subjectInfo( QSslCertificate::SubjectInfo::CommonName ) ) );
    }
    item->setIcon( 0, QgsApplication::getThemeIcon( u"/mIconCertificate.svg"_s ) );
    item->setToolTip( 0, tr( "<ul><li>Serial #: %1</li><li>Expiry date: %2</li></ul>" ).arg( cert.serialNumber(), cert.expiryDate().toString( Qt::TextDate ) ) );
    prevItem = item;
  }
  twCas->expandAll();

  return true;
}
