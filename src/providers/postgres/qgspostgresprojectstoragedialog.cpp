#include "qgspostgresprojectstoragedialog.h"

#include "qgspostgresconn.h"
#include "qgspostgresconnpool.h"
#include "qgspostgresprojectstorage.h"

#include "qgsapplication.h"
#include "qgsprojectstorage.h"
#include "qgsprojectstorageregistry.h"

#include <QMessageBox>

QgsPostgresProjectStorageDialog::QgsPostgresProjectStorageDialog( bool saving, QWidget *parent )
  : QDialog( parent )
  , mSaving( saving )
{
  setupUi( this );

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsPostgresProjectStorageDialog::onOK );

  if ( saving )
  {
    setWindowTitle( tr( "Save project to PostgreSQL" ) );
    mCboProject->setEditable( true );
  }
  else
  {
    setWindowTitle( tr( "Load project from PostgreSQL" ) );
  }

  connect( mCboConnection, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateSchemas );

  // populate connections
  mCboConnection->addItems( QgsPostgresConn::connectionList() );

  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConn::selectedConnection();
  mCboConnection->setCurrentIndex( mCboConnection->findText( toSelect ) );

  connect( mCboSchema, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsPostgresProjectStorageDialog::populateProjects );
}

QString QgsPostgresProjectStorageDialog::connectionName() const
{
  return mCboConnection->currentText();
}

QString QgsPostgresProjectStorageDialog::schemaName() const
{
  return mCboSchema->currentText();
}

QString QgsPostgresProjectStorageDialog::projectName() const
{
  return mCboProject->currentText();
}

void QgsPostgresProjectStorageDialog::populateSchemas()
{
  mCboSchema->clear();
  mCboProject->clear();

  QString name = mCboConnection->currentText();
  QgsDataSourceUri uri = QgsPostgresConn::connUri( name );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsPostgresConn *conn = QgsPostgresConnPool::instance()->acquireConnection( uri.connectionInfo( false ) );
  if ( !conn )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical( this, tr( "Error" ), tr( "Connection failed" ) + "\n" + uri.connectionInfo( false ) );
    return;
  }

  QList<QgsPostgresSchemaProperty> schemas;
  bool ok = conn->getSchemas( schemas );
  QgsPostgresConnPool::instance()->releaseConnection( conn );

  QApplication::restoreOverrideCursor();

  if ( !ok )
  {
    QMessageBox::critical( this, tr( "Error" ), tr( "Failed to get schemas" ) );
    return;
  }

  for ( const QgsPostgresSchemaProperty &schema : qAsConst( schemas ) )
  {
    mCboSchema->addItem( schema.name );
  }
}

void QgsPostgresProjectStorageDialog::populateProjects()
{
  mCboProject->clear();

  QgsPostgresProjectUri postUri;
  postUri.connInfo = QgsPostgresConn::connUri( mCboConnection->currentText() );
  postUri.schemaName = mCboSchema->currentText();
  QString uri = QgsPostgresProjectStorage::makeUri( postUri );

  QgsProjectStorage *storage = QgsApplication::projectStorageRegistry()->projectStorageFromType( "postgresql" );
  Q_ASSERT( storage );
  mCboProject->addItems( storage->listProjects( uri ) );
}

void QgsPostgresProjectStorageDialog::onOK()
{
  // check that the fields are filled in
  if ( mCboProject->currentText().isEmpty() )
    return;

  if ( mSaving )
  {
    if ( mCboProject->findText( mCboProject->currentText() ) != -1 )
    {
      int res = QMessageBox::question( this, tr( "Overwrite project" ),
                                       tr( "A project with the same name already exists. Would you like to overwrite it?" ),
                                       QMessageBox::Yes | QMessageBox::No );
      if ( res != QMessageBox::Yes )
        return;
    }
  }

  accept();
}
