//
// Created by myarjunar on 27/03/17.
//

#include <QMessageBox>
#include <QInputDialog>

#include "qgsgeonodenewconnection.h"
#include "qgsauthmanager.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgsgeonodeconnection.h"
#include "qgssettings.h"

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

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    QgsSettings settings;

    QString key = mBaseKey + '/' + connName;
    QString credentialsKey = mCredentialsBaseKey + '/' + connName;
    txtName->setText( connName );
    txtUrl->setText( settings.value( key + "/url" ).toString() );

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
  connect( txtName, SIGNAL( textChanged( QString ) ), this, SLOT( okButtonBehavior( QString ) ) );
  connect( txtUrl, SIGNAL( textChanged( QString ) ), this, SLOT( okButtonBehavior( QString ) ) );
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
    settings.remove( mBaseKey + mOriginalConnName );
    settings.remove( "qgis//" + mCredentialsBaseKey + '/' + mOriginalConnName );
    settings.sync();
  }

  QUrl url( txtUrl->text() );

  settings.setValue( key + "/url", url.toString() );

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
