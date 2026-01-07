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

#include <spatialite.h>

#include "qgis.h"
#include "qgisapp.h"
#include "qgsabstractdatabaseproviderconnection.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsfileutils.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgslogger.h"
#include "qgsproject.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderregistry.h"
#include "qgsspatialiteutils.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "moc_qgsnewspatialitelayerdialog.cpp"

QgsNewSpatialiteLayerDialog::QgsNewSpatialiteLayerDialog( QWidget *parent, Qt::WindowFlags fl, const QgsCoordinateReferenceSystem &defaultCrs )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  const auto addGeomItem = [this]( Qgis::WkbType type, const QString &sqlType ) {
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), sqlType );
  };

  addGeomItem( Qgis::WkbType::NoGeometry, QString() );
  addGeomItem( Qgis::WkbType::Point, u"POINT"_s );
  addGeomItem( Qgis::WkbType::LineString, u"LINESTRING"_s );
  addGeomItem( Qgis::WkbType::Polygon, u"POLYGON"_s );
  addGeomItem( Qgis::WkbType::MultiPoint, u"MULTIPOINT"_s );
  addGeomItem( Qgis::WkbType::MultiLineString, u"MULTILINESTRING"_s );
  addGeomItem( Qgis::WkbType::MultiPolygon, u"MULTIPOLYGON"_s );
  mGeometryTypeBox->setCurrentIndex( -1 );

  pbnFindSRID->setEnabled( false );
  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  leGeometryColumn->setEnabled( false );
  leGeometryColumn->setText( u"geometry"_s );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( u"/mActionNewAttribute.svg"_s ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( u"/mActionDeleteAttribute.svg"_s ) );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QString ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), "text" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Int ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), "integer" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Double ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), "real" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDate ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), "date" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDateTime ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), "timestamp" );

  mDatabaseComboBox->setProvider( u"spatialite"_s );

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
  connect( mButtonUp, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::moveFieldsUp );
  connect( mButtonDown, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::moveFieldsDown );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );
  mButtonUp->setEnabled( false );
  mButtonDown->setEnabled( false );
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
    return u"XYZ"_s;
  }
  else if ( !mGeometryWithZCheckBox->isChecked() && mGeometryWithMCheckBox->isChecked() )
  {
    return u"XYM"_s;
  }
  else if ( mGeometryWithZCheckBox->isChecked() && mGeometryWithMCheckBox->isChecked() )
  {
    return u"XYZM"_s;
  }

  return u"XY"_s;
}

void QgsNewSpatialiteLayerDialog::checkOk()
{
  const bool created = !leLayerName->text().isEmpty() && mGeometryTypeBox->currentIndex() != -1 && ( checkBoxPrimaryKey->isChecked() || mAttributeView->topLevelItemCount() > 0 );
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

    if ( !mNameEdit->hasFocus() )
    {
      mNameEdit->setFocus();
    }
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
  const QString sql = u"select auth_name || ':' || auth_srid from spatial_ref_sys order by srid asc"_s;

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
  mAddAttributeButton->setDisabled( name.isEmpty() || !mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewSpatialiteLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonUp->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonDown->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

bool QgsNewSpatialiteLayerDialog::createDb()
{
  QString dbPath = QFileDialog::getSaveFileName( this, tr( "New SpatiaLite Database File" ), QDir::homePath(), tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)", nullptr, QFileDialog::DontConfirmOverwrite );

  if ( dbPath.isEmpty() )
    return false;

  dbPath = QgsFileUtils::ensureFileNameHasExtension( dbPath, QStringList() << u"sqlite"_s << u"db"_s << u"sqlite3"_s << u"db3"_s << u"s3db"_s );
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
    const bool res = QgsProviderRegistry::instance()->createDb( u"spatialite"_s, dbPath, errCause );
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
    QgsProviderMetadata *md { QgsProviderRegistry::instance()->providerMetadata( u"spatialite"_s ) };
    const std::unique_ptr<QgsAbstractDatabaseProviderConnection> conn( static_cast<QgsAbstractDatabaseProviderConnection *>( md->createConnection( u"dbname='%1'"_s.arg( dbPath ), QVariantMap() ) ) );
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
  if ( !mNameEdit->text().trimmed().isEmpty() )
  {
    const QString currentFieldName = mNameEdit->text();
    bool currentFound = false;
    QTreeWidgetItemIterator it( mAttributeView );
    while ( *it )
    {
      QTreeWidgetItem *item = *it;
      if ( item->text( 0 ) == currentFieldName )
      {
        currentFound = true;
        break;
      }
      ++it;
    }

    if ( !currentFound )
    {
      if ( QMessageBox::question( this, windowTitle(), tr( "The field “%1” has not been added to the fields list. Are you sure you want to proceed and discard this field?" ).arg( currentFieldName ), QMessageBox::Ok | QMessageBox::Cancel ) != QMessageBox::Ok )
      {
        return false;
      }
    }
  }

  const QgsDataSourceUri dbUri = mDatabaseComboBox->currentConnectionUri();
  const QString dbPath = dbUri.database();

  // Build up the sql statement for creating the table
  QString sql = u"create table %1("_s.arg( quotedIdentifier( leLayerName->text() ) );
  QString delim;

  if ( checkBoxPrimaryKey->isChecked() )
  {
    sql += "pkuid integer primary key autoincrement"_L1;
    delim = u","_s;
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    sql += delim + u"%1 %2"_s.arg( quotedIdentifier( ( *it )->text( 0 ) ), ( *it )->text( 1 ) );
    delim = u","_s;
    ++it;
  }
  // complete the create table statement
  sql += ')';

  QgsDebugMsgLevel( u"Creating table in database %1"_s.arg( dbPath ), 2 );
  QgsDebugMsgLevel( sql, 2 );

  spatialite_database_unique_ptr database;
  int rc = database.open( dbPath );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, tr( "SpatiaLite Database" ), tr( "Unable to open the database: %1" ).arg( dbPath ) );
    return false;
  }

  char *errmsg = nullptr;

  // create the table
  rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errmsg );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, tr( "Error Creating SpatiaLite Table" ), tr( "Failed to create the SpatiaLite table %1. The database returned:\n%2" ).arg( leLayerName->text(), errmsg ) );
    sqlite3_free( errmsg );
    return false;
  }

  // create the geometry column and the spatial index
  if ( mGeometryTypeBox->currentIndex() != 0 )
  {
    const QString sqlAddGeom = u"select AddGeometryColumn(%1,%2,%3,%4,%5)"_s
                                 .arg( QgsSqliteUtils::quotedString( leLayerName->text() ), QgsSqliteUtils::quotedString( leGeometryColumn->text() ) )
                                 .arg( mCrsId.split( ':' ).value( 1, u"0"_s ).toInt() )
                                 .arg( QgsSqliteUtils::quotedString( selectedType() ) )
                                 .arg( QgsSqliteUtils::quotedString( selectedZM() ) );
    QgsDebugMsgLevel( sqlAddGeom, 2 );

    rc = sqlite3_exec( database.get(), sqlAddGeom.toUtf8(), nullptr, nullptr, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this, tr( "Error Creating Geometry Column" ), tr( "Failed to create the geometry column. The database returned:\n%1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
      return false;
    }

    const QString sqlCreateIndex = u"select CreateSpatialIndex(%1,%2)"_s
                                     .arg( QgsSqliteUtils::quotedString( leLayerName->text() ), QgsSqliteUtils::quotedString( leGeometryColumn->text() ) );
    QgsDebugMsgLevel( sqlCreateIndex, 2 );

    rc = sqlite3_exec( database.get(), sqlCreateIndex.toUtf8(), nullptr, nullptr, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this, tr( "Error Creating Spatial Index" ), tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
      return false;
    }
  }

  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *layer = new QgsVectorLayer( u"%1 table='%2'%3 sql="_s.arg( mDatabaseComboBox->currentConnectionUri(), leLayerName->text(), mGeometryTypeBox->currentIndex() != 0 ? u"(%1)"_s.arg( leGeometryColumn->text() ) : QString() ), leLayerName->text(), u"spatialite"_s, options );
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
    QgsDebugError( leLayerName->text() + " is an invalid layer - not loaded" );
    QMessageBox::critical( this, tr( "SpatiaLite Database" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( leLayerName->text() ) );
    delete layer;
  }

  return false;
}

QString QgsNewSpatialiteLayerDialog::quotedIdentifier( QString id )
{
  id.replace( '\"', "\"\""_L1 );
  return id.prepend( '\"' ).append( '\"' );
}

void QgsNewSpatialiteLayerDialog::showHelp()
{
  QgsHelp::openHelp( u"managing_data_source/create_layers.html#creating-a-new-spatialite-layer"_s );
}

void QgsNewSpatialiteLayerDialog::moveFieldsUp()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == 0 )
    return;

  mAttributeView->insertTopLevelItem( currentRow - 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow - 1, 0 ) );
}

void QgsNewSpatialiteLayerDialog::moveFieldsDown()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == mAttributeView->topLevelItemCount() - 1 )
    return;

  mAttributeView->insertTopLevelItem( currentRow + 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow + 1, 0 ) );
}
