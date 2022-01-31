/***************************************************************************
                         qgsnewspatialitelayerdialog.cpp
        Creates a new SpatiaLite layer. This dialog borrows heavily from
        qgsnewvectorlayerdialog.cpp
                             -------------------
    begin                : 2010-03-18
    copyright            : (C) 2010 by Gary Sherman
    email                : gsherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnewspatialitelayerdialog.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgisapp.h" // <- for theme icons
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfileutils.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsproviderconnectionmodel.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsspatialiteutils.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgsvariantutils.h"

#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>

#include <spatialite.h>

QgsNewSpatialiteLayerDialog::QgsNewSpatialiteLayerDialog( QWidget *parent, Qt::WindowFlags fl, const QgsCoordinateReferenceSystem &defaultCrs )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  const auto addGeomItem = [this]( QgsWkbTypes::Type type, const QString & sqlType )
  {
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), sqlType );
  };

  addGeomItem( QgsWkbTypes::NoGeometry, QString() );
  addGeomItem( QgsWkbTypes::Point, QStringLiteral( "POINT" ) );
  addGeomItem( QgsWkbTypes::LineString, QStringLiteral( "LINESTRING" ) );
  addGeomItem( QgsWkbTypes::Polygon, QStringLiteral( "POLYGON" ) );
  addGeomItem( QgsWkbTypes::MultiPoint, QStringLiteral( "MULTIPOINT" ) );
  addGeomItem( QgsWkbTypes::MultiLineString, QStringLiteral( "MULTILINESTRING" ) );
  addGeomItem( QgsWkbTypes::MultiPolygon, QStringLiteral( "MULTIPOLYGON" ) );
  mGeometryTypeBox->setCurrentIndex( -1 );

  pbnFindSRID->setEnabled( false );
  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  leGeometryColumn->setEnabled( false );
  leGeometryColumn->setText( QStringLiteral( "geometry" ) );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::String ), QgsVariantUtils::typeToDisplayString( QVariant::String ), "text" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Int ), QgsVariantUtils::typeToDisplayString( QVariant::Int ), "integer" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::Double ), "real" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Date ), QgsVariantUtils::typeToDisplayString( QVariant::Date ), "date" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::DateTime ), QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), "timestamp" );

  mDatabaseComboBox->setProvider( QStringLiteral( "spatialite" ) );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  // Set the SRID box to a default of WGS84
  mCrsId = defaultCrs.authid();
  leSRID->setText( defaultCrs.userFriendlyIdentifier() );

  pbnFindSRID->setEnabled( mDatabaseComboBox->count() );

  connect( mNameEdit, &QLineEdit::textChanged, this, &QgsNewSpatialiteLayerDialog::nameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewSpatialiteLayerDialog::selectionChanged );
  connect( leLayerName, &QLineEdit::textChanged, this, &QgsNewSpatialiteLayerDialog::checkOk );
  connect( checkBoxPrimaryKey, &QAbstractButton::clicked, this, &QgsNewSpatialiteLayerDialog::checkOk );
  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::mRemoveAttributeButton_clicked );
  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewSpatialiteLayerDialog::mGeometryTypeBox_currentIndexChanged );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewSpatialiteLayerDialog::mTypeBox_currentIndexChanged );
  connect( pbnFindSRID, &QPushButton::clicked, this, &QgsNewSpatialiteLayerDialog::pbnFindSRID_clicked );
  connect( toolButtonNewDatabase, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::createDb );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsNewSpatialiteLayerDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsNewSpatialiteLayerDialog::buttonBox_rejected );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewSpatialiteLayerDialog::showHelp );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );
}

void QgsNewSpatialiteLayerDialog::mGeometryTypeBox_currentIndexChanged( int index )
{
  pbnFindSRID->setEnabled( index != 0 );
  mGeometryWithZCheckBox->setEnabled( index != 0 );
  mGeometryWithMCheckBox->setEnabled( index != 0 );
  leGeometryColumn->setEnabled( index != 0 );

  checkOk();
}

void QgsNewSpatialiteLayerDialog::mTypeBox_currentIndexChanged( int index )
{
  // This isn't used since widths are irrelevant in sqlite3
  switch ( index )
  {
    case 0: // Text data
    case 1: // Whole number
    case 2: // Decimal number
    case 3: // Date
    case 4: // Date time
    default:
      break;
  }
}

QString QgsNewSpatialiteLayerDialog::selectedType() const
{
  return mGeometryTypeBox->currentData( Qt::UserRole ).toString();
}

QString QgsNewSpatialiteLayerDialog::selectedZM() const
{
  if ( mGeometryWithZCheckBox->isChecked() && !mGeometryWithMCheckBox->isChecked() )
  {
    return QStringLiteral( "XYZ" );
  }
  else if ( !mGeometryWithZCheckBox->isChecked() && mGeometryWithMCheckBox->isChecked() )
  {
    return QStringLiteral( "XYM" );
  }
  else if ( mGeometryWithZCheckBox->isChecked() && mGeometryWithMCheckBox->isChecked() )
  {
    return QStringLiteral( "XYZM" );
  }

  return QStringLiteral( "XY" );
}

void QgsNewSpatialiteLayerDialog::checkOk()
{
  const bool created  = !leLayerName->text().isEmpty() && mGeometryTypeBox->currentIndex() != -1 &&
                        ( checkBoxPrimaryKey->isChecked() || mAttributeView->topLevelItemCount() > 0 );
  mOkButton->setEnabled( created );
}

void QgsNewSpatialiteLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mNameEdit->text().isEmpty() )
  {
    const QString myName = mNameEdit->text();
    //use userrole to avoid translated type string
    const QString myType = mTypeBox->currentData( Qt::UserRole ).toString();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType ) );

    checkOk();

    mNameEdit->clear();
  }
}

void QgsNewSpatialiteLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewSpatialiteLayerDialog::pbnFindSRID_clicked()
{
  const QgsDataSourceUri dbUri = mDatabaseComboBox->currentConnectionUri();
  const QString dbPath = dbUri.database();

  // first get list of supported SRID from the selected SpatiaLite database
  // to build filter for projection selector
  sqlite3_database_unique_ptr database;
  bool status = true;
  int rc = database.open_v2( dbPath, SQLITE_OPEN_READONLY, nullptr );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, tr( "SpatiaLite Database" ), tr( "Unable to open the database" ) );
    return;
  }

  // load up the srid table
  sqlite3_statement_unique_ptr statement;
  const QString sql = QStringLiteral( "select auth_name || ':' || auth_srid from spatial_ref_sys order by srid asc" );

  QSet<QString> myCRSs;

  statement = database.prepare( sql, rc );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    // get the first row of the result set
    while ( sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      myCRSs.insert( statement.columnAsText( 0 ) );
    }
  }
  else
  {
    // XXX query failed -- warn the user somehow
    QMessageBox::warning( nullptr, tr( "Error" ), tr( "Failed to load SRIDS: %1" ).arg( database.errorMessage() ) );
    status = false;
  }

  if ( status )
  {
    // prepare projection selector
    QgsProjectionSelectionDialog *mySelector = new QgsProjectionSelectionDialog( this );
    mySelector->setOgcWmsCrsFilter( myCRSs );
    mySelector->setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( mCrsId ) );

    if ( mySelector->exec() )
    {
      const QgsCoordinateReferenceSystem srs = mySelector->crs();
      const QString crsId = srs.authid();
      if ( crsId != mCrsId )
      {
        mCrsId = crsId;
        leSRID->setText( srs.userFriendlyIdentifier() );
      }
    }
    delete mySelector;
  }
}

void QgsNewSpatialiteLayerDialog::nameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || ! mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewSpatialiteLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

bool QgsNewSpatialiteLayerDialog::createDb()
{
  QString dbPath = QFileDialog::getSaveFileName( this, tr( "New SpatiaLite Database File" ),
                   QDir::homePath(),
                   tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)", nullptr, QFileDialog::DontConfirmOverwrite );

  if ( dbPath.isEmpty() )
    return false;

  dbPath = QgsFileUtils::ensureFileNameHasExtension( dbPath, QStringList() << QStringLiteral( "sqlite" ) << QStringLiteral( "db" ) << QStringLiteral( "sqlite3" )
           << QStringLiteral( "db3" ) << QStringLiteral( "s3db" ) );
  QFile newDb( dbPath );
  if ( newDb.exists() )
  {
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( tr( "New SpatiaLite Layer" ) );
    msgBox.setText( tr( "The file already exists. Do you want to overwrite the existing file with a new database or add a new layer to it?" ) );
    QPushButton *overwriteButton = msgBox.addButton( tr( "Overwrite" ), QMessageBox::ActionRole );
    QPushButton *addNewLayerButton = msgBox.addButton( tr( "Add New Layer" ), QMessageBox::ActionRole );
    msgBox.setStandardButtons( QMessageBox::Cancel );
    msgBox.setDefaultButton( addNewLayerButton );
    const int ret = msgBox.exec();
    if ( ret == QMessageBox::Cancel )
    {
      return false;
    }

    if ( msgBox.clickedButton() == overwriteButton )
    {
      newDb.remove();
    }
  }

  if ( !newDb.exists() )
  {
    QString errCause;
    const bool res = QgsProviderRegistry::instance()->createDb( QStringLiteral( "spatialite" ), dbPath, errCause );
    if ( !res )
    {
      QMessageBox::warning( nullptr, tr( "SpatiaLite Database" ), errCause );
      pbnFindSRID->setEnabled( false );
    }
  }

  const QFileInfo fi( newDb );
  if ( !fi.exists() )
  {
    pbnFindSRID->setEnabled( false );
  }
  else
  {
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "spatialite" ) ) };
    const std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( QStringLiteral( "dbname='%1'" ).arg( dbPath ), QVariantMap() ) ) );
    if ( conn )
    {
      md->saveConnection( conn.get(), fi.fileName() );
      mDatabaseComboBox->setConnection( fi.fileName() );
      pbnFindSRID->setEnabled( true );
      return true;
    }
  }

  return false;
}

void QgsNewSpatialiteLayerDialog::buttonBox_accepted()
{
  if ( apply() )
    accept();
}

void QgsNewSpatialiteLayerDialog::buttonBox_rejected()
{
  reject();
}

bool QgsNewSpatialiteLayerDialog::apply()
{
  const QgsDataSourceUri dbUri = mDatabaseComboBox->currentConnectionUri();
  const QString dbPath = dbUri.database();

  // Build up the sql statement for creating the table
  QString sql = QStringLiteral( "create table %1(" ).arg( quotedIdentifier( leLayerName->text() ) );
  QString delim;

  if ( checkBoxPrimaryKey->isChecked() )
  {
    sql += QLatin1String( "pkuid integer primary key autoincrement" );
    delim = QStringLiteral( "," );
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    sql += delim + QStringLiteral( "%1 %2" ).arg( quotedIdentifier( ( *it )->text( 0 ) ), ( *it )->text( 1 ) );
    delim = QStringLiteral( "," );
    ++it;
  }
  // complete the create table statement
  sql += ')';

  QgsDebugMsg( QStringLiteral( "Creating table in database %1" ).arg( dbPath ) );
  QgsDebugMsg( sql );

  spatialite_database_unique_ptr database;
  int rc = database.open( dbPath );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this,
                          tr( "SpatiaLite Database" ),
                          tr( "Unable to open the database: %1" ).arg( dbPath ) );
    return false;
  }

  char *errmsg = nullptr;

  // create the table
  rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errmsg );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this,
                          tr( "Error Creating SpatiaLite Table" ),
                          tr( "Failed to create the SpatiaLite table %1. The database returned:\n%2" ).arg( leLayerName->text(), errmsg ) );
    sqlite3_free( errmsg );
    return false;
  }

  // create the geometry column and the spatial index
  if ( mGeometryTypeBox->currentIndex() != 0 )
  {
    const QString sqlAddGeom = QStringLiteral( "select AddGeometryColumn(%1,%2,%3,%4,%5)" )
                               .arg( QgsSqliteUtils::quotedString( leLayerName->text() ),
                                     QgsSqliteUtils::quotedString( leGeometryColumn->text() ) )
                               .arg( mCrsId.split( ':' ).value( 1, QStringLiteral( "0" ) ).toInt() )
                               .arg( QgsSqliteUtils::quotedString( selectedType() ) )
                               .arg( QgsSqliteUtils::quotedString( selectedZM() ) );
    QgsDebugMsg( sqlAddGeom );

    rc = sqlite3_exec( database.get(), sqlAddGeom.toUtf8(), nullptr, nullptr, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this,
                            tr( "Error Creating Geometry Column" ),
                            tr( "Failed to create the geometry column. The database returned:\n%1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
      return false;
    }

    const QString sqlCreateIndex = QStringLiteral( "select CreateSpatialIndex(%1,%2)" )
                                   .arg( QgsSqliteUtils::quotedString( leLayerName->text() ),
                                         QgsSqliteUtils::quotedString( leGeometryColumn->text() ) );
    QgsDebugMsg( sqlCreateIndex );

    rc = sqlite3_exec( database.get(), sqlCreateIndex.toUtf8(), nullptr, nullptr, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this,
                            tr( "Error Creating Spatial Index" ),
                            tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
      return false;
    }
  }

  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "%1 table='%2'%3 sql=" )
      .arg( mDatabaseComboBox->currentConnectionUri(),
            leLayerName->text(),
            mGeometryTypeBox->currentIndex() != 0 ? QStringLiteral( "(%1)" ).arg( leGeometryColumn->text() ) : QString() ),
      leLayerName->text(), QStringLiteral( "spatialite" ), options );
  if ( layer->isValid() )
  {
    // Reload connections to refresh browser panel
    QgisApp::instance()->reloadConnections();

    // register this layer with the central layers registry
    QList<QgsMapLayer *> myList;
    myList << layer;
    //addMapLayers returns a list of all successfully added layers
    //so we compare that to our original list.
    if ( myList == QgsProject::instance()->addMapLayers( myList ) )
      return true;
  }
  else
  {
    QgsDebugMsg( leLayerName->text() + " is an invalid layer - not loaded" );
    QMessageBox::critical( this, tr( "SpatiaLite Database" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( leLayerName->text() ) );
    delete layer;
  }

  return false;
}

QString QgsNewSpatialiteLayerDialog::quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

void QgsNewSpatialiteLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-spatialite-layer" ) );
}
