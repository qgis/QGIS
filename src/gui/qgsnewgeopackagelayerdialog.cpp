/***************************************************************************
                         qgsnewgeopackagelayerdialog.cpp

                             -------------------
    begin                : April 2016
    copyright            : (C) 2016 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnewgeopackagelayerdialog.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgenericprojectionselector.h"
#include "qgscrscache.h"

#include "qgslogger.h"

#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QLibrary>

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <gdal_version.h>
#include <cpl_error.h>
#include <cpl_string.h>

#if defined(GDAL_COMPUTE_VERSION) && GDAL_VERSION_NUM >= GDAL_COMPUTE_VERSION(2,0,0)
#define SUPPORT_GEOMETRY_LESS
#define SUPPORT_CURVE_GEOMETRIES
#define SUPPORT_INTEGER64
#define SUPPORT_SPATIAL_INDEX
#define SUPPORT_IDENTIFIER_DESCRIPTION
#define SUPPORT_FIELD_WIDTH
#endif

QgsNewGeoPackageLayerDialog::QgsNewGeoPackageLayerDialog( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
    , mOkButton( nullptr )
    , mTableNameEdited( false )
    , mLayerIdentifierEdited( false )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/NewGeoPackageLayer/geometry" ).toByteArray() );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.svg" ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.svg" ) );

#ifdef SUPPORT_GEOMETRY_LESS
  mGeometryTypeBox->addItem( tr( "Non spatial" ), wkbNone );
#endif
  mGeometryTypeBox->addItem( tr( "Point" ), wkbPoint );
  mGeometryTypeBox->addItem( tr( "Line" ), wkbLineString );
  mGeometryTypeBox->addItem( tr( "Polygon" ), wkbPolygon );
  mGeometryTypeBox->addItem( tr( "Multi point" ), wkbMultiPoint );
  mGeometryTypeBox->addItem( tr( "Multi line" ), wkbMultiLineString );
  mGeometryTypeBox->addItem( tr( "Multi polygon" ), wkbMultiPolygon );
#ifdef SUPPORT_CURVE_GEOMETRIES
#if 0
  // QGIS always create CompoundCurve and there's no real interest of having just CircularString. CompoundCurve are more useful
  mGeometryTypeBox->addItem( tr( "Circular string" ), wkbCircularString );
#endif
  mGeometryTypeBox->addItem( tr( "Compound curve" ), wkbCompoundCurve );
  mGeometryTypeBox->addItem( tr( "Curve polygon" ), wkbCurvePolygon );
  mGeometryTypeBox->addItem( tr( "Multi curve" ), wkbMultiCurve );
  mGeometryTypeBox->addItem( tr( "Multi surface" ), wkbMultiSurface );
#endif

#ifdef SUPPORT_GEOMETRY_LESS
  mGeometryColumnEdit->setEnabled( false );
  mCheckBoxCreateSpatialIndex->setEnabled( false );
  mCrsSelector->setEnabled( false );
#endif

  mFieldTypeBox->addItem( tr( "Text data" ), "text" );
  mFieldTypeBox->addItem( tr( "Whole number (integer)" ), "integer" );
#ifdef SUPPORT_INTEGER64
  mFieldTypeBox->addItem( tr( "Whole number (integer 64 bit)" ), "integer64" );
#endif
  mFieldTypeBox->addItem( tr( "Decimal number (real)" ), "real" );
  mFieldTypeBox->addItem( tr( "Date" ), "date" );
  mFieldTypeBox->addItem( tr( "Date&time" ), "datetime" );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  // Set the SRID box to a default of WGS84
  QgsCoordinateReferenceSystem defaultCrs = QgsCRSCache::instance()->crsByOgcWmsCrs( settings.value( "/Projections/layerDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString() );
  defaultCrs.validate();
  mCrsSelector->setCrs( defaultCrs );

  connect( mFieldNameEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( fieldNameChanged( QString ) ) );
  connect( mAttributeView, SIGNAL( itemSelectionChanged() ), this, SLOT( selectionChanged() ) );
  connect( mTableNameEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( checkOk() ) );
  connect( mDatabaseEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( checkOk() ) );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

#ifndef SUPPORT_SPATIAL_INDEX
  mCheckBoxCreateSpatialIndex->hide();
  mCheckBoxCreateSpatialIndex->setChecked( false );
#else
  mCheckBoxCreateSpatialIndex->setChecked( true );
#endif

#ifndef SUPPORT_IDENTIFIER_DESCRIPTION
  mLayerIdentifierLabel->hide();
  mLayerIdentifierEdit->hide();
  mLayerDescriptionLabel->hide();
  mLayerDescriptionEdit->hide();
#endif

#ifndef SUPPORT_FIELD_WIDTH
  mFieldLengthLabel->hide();
  mFieldLengthEdit->hide();
#endif
}

QgsNewGeoPackageLayerDialog::~QgsNewGeoPackageLayerDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/NewGeoPackageLayer/geometry", saveGeometry() );
}

void QgsNewGeoPackageLayerDialog::on_mFieldTypeBox_currentIndexChanged( int )
{
  QString myType = mFieldTypeBox->itemData( mFieldTypeBox->currentIndex(), Qt::UserRole ).toString();
  mFieldLengthEdit->setEnabled( myType == "text" );
  if ( myType != "text" )
    mFieldLengthEdit->setText( "" );
}


void QgsNewGeoPackageLayerDialog::on_mGeometryTypeBox_currentIndexChanged( int )
{
  OGRwkbGeometryType geomType = static_cast<OGRwkbGeometryType>
                                ( mGeometryTypeBox->itemData( mGeometryTypeBox->currentIndex(), Qt::UserRole ).toInt() );
  bool isSpatial = geomType != wkbNone;
  mGeometryColumnEdit->setEnabled( isSpatial );
  mCheckBoxCreateSpatialIndex->setEnabled( isSpatial );
  mCrsSelector->setEnabled( isSpatial );
}

void QgsNewGeoPackageLayerDialog::on_mSelectDatabaseButton_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "Select existing or create new GeoPackage Database File" ),
                     QDir::homePath(),
                     tr( "GeoPackage" ) + " (*.gpkg)",
                     nullptr,
                     QFileDialog::DontConfirmOverwrite );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.endsWith( ".gpkg", Qt::CaseInsensitive ) )
  {
    fileName += ".gpkg";
  }

  mDatabaseEdit->setText( fileName );
}

void QgsNewGeoPackageLayerDialog::on_mDatabaseEdit_textChanged( const QString& text )
{
  if ( !text.isEmpty() && !mTableNameEdited )
  {
    QFileInfo fileInfo( text );
    mTableNameEdit->setText( fileInfo.baseName() );
  }
}

void QgsNewGeoPackageLayerDialog::on_mTableNameEdit_textChanged( const QString& text )
{
  mTableNameEdited = !text.isEmpty();
  if ( !text.isEmpty() && !mLayerIdentifierEdited )
  {
    mLayerIdentifierEdit->setText( text );
  }
}

void QgsNewGeoPackageLayerDialog::on_mTableNameEdit_textEdited( const QString& text )
{
  // Remember if the user explicitly defined a name
  mTableNameEdited = !text.isEmpty();
}

void QgsNewGeoPackageLayerDialog::on_mLayerIdentifierEdit_textEdited( const QString& text )
{
  // Remember if the user explicitly defined a name
  mLayerIdentifierEdited = !text.isEmpty();
}

void QgsNewGeoPackageLayerDialog::checkOk()
{
  bool ok = !mDatabaseEdit->text().isEmpty() &&
            !mTableNameEdit->text().isEmpty();
  mOkButton->setEnabled( ok );
}

void QgsNewGeoPackageLayerDialog::on_mAddAttributeButton_clicked()
{
  if ( !mFieldNameEdit->text().isEmpty() )
  {
    QString myName = mFieldNameEdit->text();
    if ( myName == mFeatureIdColumnEdit->text() )
    {
      QMessageBox::critical( this, tr( "Invalid field name" ), tr( "The field cannot have the same name as the feature identifier" ) );
      return;
    }

    //use userrole to avoid translated type string
    QString myType = mFieldTypeBox->itemData( mFieldTypeBox->currentIndex(), Qt::UserRole ).toString();
    QString length = mFieldLengthEdit->text();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << length ) );

    checkOk();

    mFieldNameEdit->clear();
  }
}

void QgsNewGeoPackageLayerDialog::on_mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewGeoPackageLayerDialog::fieldNameChanged( const QString& name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || ! mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewGeoPackageLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

void QgsNewGeoPackageLayerDialog::on_buttonBox_accepted()
{
  if ( apply() )
    accept();
}

void QgsNewGeoPackageLayerDialog::on_buttonBox_rejected()
{
  reject();
}

bool QgsNewGeoPackageLayerDialog::apply()
{
  QString fileName( mDatabaseEdit->text() );
  bool createNewDb = false;
  if ( QFile( fileName ).exists( fileName ) )
  {
    QMessageBox msgBox;
    msgBox.setIcon( QMessageBox::Question );
    msgBox.setWindowTitle( tr( "The file already exists." ) );
    msgBox.setText( tr( "Do you want to overwrite the existing file with a new database or add a new layer to it?" ) );
    QPushButton *overwriteButton = msgBox.addButton( tr( "Overwrite" ), QMessageBox::ActionRole );
    QPushButton *addNewLayerButton = msgBox.addButton( tr( "Add new layer" ), QMessageBox::ActionRole );
    msgBox.setStandardButtons( QMessageBox::Cancel );
    msgBox.setDefaultButton( addNewLayerButton );
    bool overwrite = false;
    bool cancel = false;
    if ( property( "hideDialogs" ).toBool() )
    {
      overwrite = property( "question_existing_db_answer_overwrite" ).toBool();
      if ( !overwrite )
        cancel = !property( "question_existing_db_answer_add_new_layer" ).toBool();
    }
    else
    {
      int ret = msgBox.exec();
      if ( ret == QMessageBox::Cancel )
        cancel = true;
      if ( msgBox.clickedButton() == overwriteButton )
        overwrite = true;
    }
    if ( cancel )
    {
      return false;
    }
    if ( overwrite )
    {
      QFile( fileName ).remove();
      createNewDb = true;
    }
  }
  else
  {
    createNewDb = true;
  }

  OGRSFDriverH hGpkgDriver = OGRGetDriverByName( "GPKG" );
  if ( !hGpkgDriver )
  {
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "Layer creation failed" ),
                             tr( "GeoPackage driver not found" ) );
    return false;
  }

  OGRDataSourceH hDS = nullptr;
  if ( createNewDb )
  {
    hDS = OGR_Dr_CreateDataSource( hGpkgDriver, fileName.toUtf8().constData(), nullptr );
    if ( !hDS )
    {
      QString msg( tr( "Creation of database failed (OGR error:%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "Layer creation failed" ), msg );
      return false;
    }
  }
  else
  {
    OGRSFDriverH hDriver = nullptr;
    hDS = OGROpen( fileName.toUtf8().constData(), true, &hDriver );
    if ( !hDS )
    {
      QString msg( tr( "Opening of database failed (OGR error:%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "Layer creation failed" ), msg );
      return false;
    }
    if ( hDriver != hGpkgDriver )
    {
      QString msg( tr( "Opening of file succeeded, but this is not a GeoPackage database" ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "Layer creation failed" ), msg );
      OGR_DS_Destroy( hDS );
      return false;
    }
  }

  QString tableName( mTableNameEdit->text() );

  bool overwriteTable = false;
  if ( OGR_DS_GetLayerByName( hDS, tableName.toUtf8().constData() ) != nullptr )
  {
    if ( property( "hideDialogs" ).toBool() )
    {
      overwriteTable = property( "question_existing_layer_answer_overwrite" ).toBool();
    }
    else if ( QMessageBox::question( this, tr( "Existing layer" ),
                                     tr( "A table with the same name already exists. Do you want to overwrite it?" ),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
    {
      overwriteTable = true;
    }

    if ( !overwriteTable )
    {
      OGR_DS_Destroy( hDS );
      return false;
    }
  }

  QString layerIdentifier( mLayerIdentifierEdit->text() );
  QString layerDescription( mLayerDescriptionEdit->text() );

  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>
                               ( mGeometryTypeBox->itemData( mGeometryTypeBox->currentIndex(), Qt::UserRole ).toInt() );

  OGRSpatialReferenceH hSRS = nullptr;
  // consider spatial reference system of the layer
  int crsId = mCrsSelector->crs().srsid();
  if ( wkbType != wkbNone && crsId > 0 )
  {
    QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsBySrsId( crsId );
    QString srsWkt = srs.toWkt();
    hSRS = OSRNewSpatialReference( srsWkt.toLocal8Bit().data() );
  }

  // Set options
  char **options = nullptr;

  if ( overwriteTable )
    options = CSLSetNameValue( options, "OVERWRITE", "YES" );
  if ( !layerIdentifier.isEmpty() )
    options = CSLSetNameValue( options, "IDENTIFIER", layerIdentifier.toUtf8().constData() );
  if ( !layerDescription.isEmpty() )
    options = CSLSetNameValue( options, "DESCRIPTION", layerDescription.toUtf8().constData() );

  QString featureId( mFeatureIdColumnEdit->text() );
  if ( !featureId.isEmpty() )
    options = CSLSetNameValue( options, "FID", featureId.toUtf8().constData() );

  QString geometryColumn( mGeometryColumnEdit->text() );
  if ( wkbType != wkbNone && !geometryColumn.isEmpty() )
    options = CSLSetNameValue( options, "GEOMETRY_COLUMN", geometryColumn.toUtf8().constData() );

#ifdef SUPPORT_SPATIAL_INDEX
  if ( wkbType != wkbNone )
    options = CSLSetNameValue( options, "SPATIAL_INDEX", mCheckBoxCreateSpatialIndex->isChecked() ? "YES" : "NO" );
#endif

  OGRLayerH hLayer = OGR_DS_CreateLayer( hDS, tableName.toUtf8().constData(), hSRS, wkbType, options );
  CSLDestroy( options );
  if ( hSRS != nullptr )
    OSRRelease( hSRS );
  if ( hLayer == nullptr )
  {
    QString msg( tr( "Creation of layer failed (OGR error:%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "Layer creation failed" ), msg );
    OGR_DS_Destroy( hDS );
    return false;
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QString fieldName(( *it )->text( 0 ) );
    QString fieldType(( *it )->text( 1 ) );
    QString fieldWidth(( *it )->text( 2 ) );

    OGRFieldType ogrType( OFTString );
    if ( fieldType == "text" )
      ogrType = OFTString;
    else if ( fieldType == "integer" )
      ogrType = OFTInteger;
#ifdef SUPPORT_INTEGER64
    else if ( fieldType == "integer64" )
      ogrType = OFTInteger64;
#endif
    else if ( fieldType == "real" )
      ogrType = OFTReal;
    else if ( fieldType == "date" )
      ogrType = OFTDate;
    else if ( fieldType == "datetime" )
      ogrType = OFTDateTime;

    int ogrWidth = fieldWidth.toInt();

    OGRFieldDefnH fld = OGR_Fld_Create( fieldName.toUtf8().constData(), ogrType );
    OGR_Fld_SetWidth( fld, ogrWidth );

    if ( OGR_L_CreateField( hLayer, fld, true ) != OGRERR_NONE )
    {
      if ( !property( "hideDialogs" ).toBool() )
      {
        QMessageBox::critical( this, tr( "Layer creation failed" ),
                               tr( "Creation of field %1 failed (OGR error: %2)" )
                               .arg( fieldName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      }
      OGR_Fld_Destroy( fld );
      OGR_DS_Destroy( hDS );
      return false;
    }
    OGR_Fld_Destroy( fld );

    ++it;
  }

  // In GDAL >= 2.0, the driver implements a defered creation strategy, so
  // issue a command that will force table creation
  CPLErrorReset();
  OGR_L_ResetReading( hLayer );
  if ( CPLGetLastErrorType() != CE_None )
  {
    QString msg( tr( "Creation of layer failed (OGR error:%1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "Layer creation failed" ), msg );
    OGR_DS_Destroy( hDS );
    return false;
  }

  OGR_DS_Destroy( hDS );

  QString uri( QString( "%1|layername=%2" ).arg( mDatabaseEdit->text(), tableName ) );
  QString userVisiblelayerName( layerIdentifier.isEmpty() ? tableName : layerIdentifier );
  QgsVectorLayer *layer = new QgsVectorLayer( uri, userVisiblelayerName, "ogr" );
  if ( layer->isValid() )
  {
    // register this layer with the central layers registry
    QList<QgsMapLayer *> myList;
    myList << layer;
    //addMapLayers returns a list of all successfully added layers
    //so we compare that to our original list.
    if ( myList == QgsMapLayerRegistry::instance()->addMapLayers( myList ) )
      return true;
  }
  else
  {
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "Invalid Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( tableName ) );
    delete layer;
  }

  return false;
}

