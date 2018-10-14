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
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgssettings.h"
#include "qgshelp.h"
#include "qgsogrutils.h"
#include "qgsgui.h"

#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QLibrary>

#include <ogr_api.h>
#include <ogr_srs_api.h>
#include <gdal_version.h>
#include <cpl_error.h>
#include <cpl_string.h>

#define DEFAULT_OGR_FID_COLUMN_TITLE "fid" // default value from OGR

QgsNewGeoPackageLayerDialog::QgsNewGeoPackageLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewGeoPackageLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewGeoPackageLayerDialog::mRemoveAttributeButton_clicked );
  connect( mFieldTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewGeoPackageLayerDialog::mFieldTypeBox_currentIndexChanged );
  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewGeoPackageLayerDialog::mGeometryTypeBox_currentIndexChanged );
  connect( mTableNameEdit, &QLineEdit::textChanged, this, &QgsNewGeoPackageLayerDialog::mTableNameEdit_textChanged );
  connect( mTableNameEdit, &QLineEdit::textEdited, this, &QgsNewGeoPackageLayerDialog::mTableNameEdit_textEdited );
  connect( mLayerIdentifierEdit, &QLineEdit::textEdited, this, &QgsNewGeoPackageLayerDialog::mLayerIdentifierEdit_textEdited );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsNewGeoPackageLayerDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsNewGeoPackageLayerDialog::buttonBox_rejected );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewGeoPackageLayerDialog::showHelp );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );

  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconTableLayer.svg" ) ), tr( "No geometry" ), wkbNone );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "Point" ), wkbPoint );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "Line" ), wkbLineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "Polygon" ), wkbPolygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPointLayer.svg" ) ), tr( "MultiPoint" ), wkbMultiPoint );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "MultiLine" ), wkbMultiLineString );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "MultiPolygon" ), wkbMultiPolygon );

#if 0
  // QGIS always create CompoundCurve and there's no real interest of having just CircularString. CompoundCurve are more useful
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "CircularString" ), wkbCircularString );
#endif
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "CompoundCurve" ), wkbCompoundCurve );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "CurvePolygon" ), wkbCurvePolygon );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconLineLayer.svg" ) ), tr( "MultiCurve" ), wkbMultiCurve );
  mGeometryTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconPolygonLayer.svg" ) ), tr( "MultiSurface" ), wkbMultiSurface );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  mGeometryColumnEdit->setEnabled( false );
  mGeometryColumnEdit->setText( "geometry" );
  mFeatureIdColumnEdit->setPlaceholderText( DEFAULT_OGR_FID_COLUMN_TITLE );
  mCheckBoxCreateSpatialIndex->setEnabled( false );
  mCrsSelector->setEnabled( false );

  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) ), tr( "Text data" ), "text" );
  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) ), tr( "Whole number (integer)" ), "integer" );
  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldInteger.svg" ) ), tr( "Whole number (integer 64 bit)" ), "integer64" );
  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldFloat.svg" ) ), tr( "Decimal number (real)" ), "real" );
  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDate.svg" ) ), tr( "Date" ), "date" );
  mFieldTypeBox->addItem( QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldDateTime.svg" ) ), tr( "Date&time" ), "datetime" );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  connect( mFieldNameEdit, &QLineEdit::textChanged, this, &QgsNewGeoPackageLayerDialog::fieldNameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewGeoPackageLayerDialog::selectionChanged );
  connect( mTableNameEdit, &QLineEdit::textChanged, this, &QgsNewGeoPackageLayerDialog::checkOk );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

  mCheckBoxCreateSpatialIndex->setChecked( true );

  QgsSettings settings;
  mDatabase->setStorageMode( QgsFileWidget::SaveFile );
  mDatabase->setFilter( tr( "GeoPackage" ) + " (*.gpkg)" );
  mDatabase->setDialogTitle( tr( "Select Existing or Create a New GeoPackage Database File…" ) );
  mDatabase->setDefaultRoot( settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString() );
  mDatabase->setConfirmOverwrite( false );
  connect( mDatabase, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), tmplFileInfo.absolutePath() );
    if ( !filePath.isEmpty() && !mTableNameEdited )
    {
      QFileInfo fileInfo( filePath );
      mTableNameEdit->setText( fileInfo.baseName() );
    }
    checkOk();
  } );
}

void QgsNewGeoPackageLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsSelector->setCrs( crs );
}

void QgsNewGeoPackageLayerDialog::lockDatabasePath()
{
  mDatabase->setReadOnly( true );
}

void QgsNewGeoPackageLayerDialog::mFieldTypeBox_currentIndexChanged( int )
{
  QString myType = mFieldTypeBox->currentData( Qt::UserRole ).toString();
  mFieldLengthEdit->setEnabled( myType == QLatin1String( "text" ) );
  if ( myType != QLatin1String( "text" ) )
    mFieldLengthEdit->clear();
}


void QgsNewGeoPackageLayerDialog::mGeometryTypeBox_currentIndexChanged( int )
{
  OGRwkbGeometryType geomType = static_cast<OGRwkbGeometryType>
                                ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );
  bool isSpatial = geomType != wkbNone;
  mGeometryWithZCheckBox->setEnabled( isSpatial );
  mGeometryWithMCheckBox->setEnabled( isSpatial );
  mGeometryColumnEdit->setEnabled( isSpatial );
  mCheckBoxCreateSpatialIndex->setEnabled( isSpatial );
  mCrsSelector->setEnabled( isSpatial );
}

void QgsNewGeoPackageLayerDialog::mTableNameEdit_textChanged( const QString &text )
{
  mTableNameEdited = !text.isEmpty();
  if ( !text.isEmpty() && !mLayerIdentifierEdited )
  {
    mLayerIdentifierEdit->setText( text );
  }
}

void QgsNewGeoPackageLayerDialog::mTableNameEdit_textEdited( const QString &text )
{
  // Remember if the user explicitly defined a name
  mTableNameEdited = !text.isEmpty();
}

void QgsNewGeoPackageLayerDialog::mLayerIdentifierEdit_textEdited( const QString &text )
{
  // Remember if the user explicitly defined a name
  mLayerIdentifierEdited = !text.isEmpty();
}

void QgsNewGeoPackageLayerDialog::checkOk()
{
  bool ok = !mDatabase->filePath().isEmpty() &&
            !mTableNameEdit->text().isEmpty();
  mOkButton->setEnabled( ok );
}

void QgsNewGeoPackageLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mFieldNameEdit->text().isEmpty() )
  {
    QString myName = mFieldNameEdit->text();
    const QString featureId = mFeatureIdColumnEdit->text().isEmpty() ? DEFAULT_OGR_FID_COLUMN_TITLE : mFeatureIdColumnEdit->text();
    if ( myName.compare( featureId, Qt::CaseInsensitive ) == 0 )
    {
      QMessageBox::critical( this, tr( "Add Field" ), tr( "The field cannot have the same name as the feature identifier." ) );
      return;
    }

    //use userrole to avoid translated type string
    QString myType = mFieldTypeBox->currentData( Qt::UserRole ).toString();
    QString length = mFieldLengthEdit->text();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << length ) );

    checkOk();

    mFieldNameEdit->clear();
  }
}

void QgsNewGeoPackageLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewGeoPackageLayerDialog::fieldNameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || ! mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewGeoPackageLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

void QgsNewGeoPackageLayerDialog::buttonBox_accepted()
{
  if ( apply() )
    accept();
}

void QgsNewGeoPackageLayerDialog::buttonBox_rejected()
{
  reject();
}

bool QgsNewGeoPackageLayerDialog::apply()
{
  QString fileName( mDatabase->filePath() );
  if ( !fileName.endsWith( QLatin1String( ".gpkg" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".gpkg" );

  bool createNewDb = false;

  if ( QFile( fileName ).exists( fileName ) )
  {
    bool overwrite = false;

    switch ( mBehavior )
    {
      case Prompt:
      {
        QMessageBox msgBox;
        msgBox.setIcon( QMessageBox::Question );
        msgBox.setWindowTitle( tr( "New GeoPackage Layer" ) );
        msgBox.setText( tr( "The File already exists. Do you want to overwrite the existing file with a new database or add a new layer to it?" ) );
        QPushButton *overwriteButton = msgBox.addButton( tr( "Overwrite" ), QMessageBox::ActionRole );
        QPushButton *addNewLayerButton = msgBox.addButton( tr( "Add new layer" ), QMessageBox::ActionRole );
        msgBox.setStandardButtons( QMessageBox::Cancel );
        msgBox.setDefaultButton( addNewLayerButton );
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
        break;
      }

      case Overwrite:
        overwrite = true;
        break;

      case AddNewLayer:
        overwrite = false;
        break;
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
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ),
                             tr( "Layer creation failed. GeoPackage driver not found." ) );
    return false;
  }

  gdal::ogr_datasource_unique_ptr hDS;
  if ( createNewDb )
  {
    hDS.reset( OGR_Dr_CreateDataSource( hGpkgDriver, fileName.toUtf8().constData(), nullptr ) );
    if ( !hDS )
    {
      QString msg( tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
      return false;
    }
  }
  else
  {
    OGRSFDriverH hDriver = nullptr;
    hDS.reset( OGROpen( fileName.toUtf8().constData(), true, &hDriver ) );
    if ( !hDS )
    {
      QString msg( tr( "Opening of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
      return false;
    }
    if ( hDriver != hGpkgDriver )
    {
      QString msg( tr( "Opening of file succeeded, but this is not a GeoPackage database." ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
      return false;
    }
  }

  QString tableName( mTableNameEdit->text() );

  bool overwriteTable = false;
  if ( OGR_DS_GetLayerByName( hDS.get(), tableName.toUtf8().constData() ) )
  {
    if ( property( "hideDialogs" ).toBool() )
    {
      overwriteTable = property( "question_existing_layer_answer_overwrite" ).toBool();
    }
    else if ( QMessageBox::question( this, tr( "New GeoPackage Layer" ),
                                     tr( "A table with the same name already exists. Do you want to overwrite it?" ),
                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
    {
      overwriteTable = true;
    }

    if ( !overwriteTable )
    {
      return false;
    }
  }

  QString layerIdentifier( mLayerIdentifierEdit->text() );
  QString layerDescription( mLayerDescriptionEdit->text() );

  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>
                               ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  // z-coordinate & m-value.
  if ( mGeometryWithZCheckBox->isChecked() )
    wkbType = OGR_GT_SetZ( wkbType );

  if ( mGeometryWithMCheckBox->isChecked() )
    wkbType = OGR_GT_SetM( wkbType );

  OGRSpatialReferenceH hSRS = nullptr;
  // consider spatial reference system of the layer
  QgsCoordinateReferenceSystem srs = mCrsSelector->crs();
  if ( wkbType != wkbNone && srs.isValid() )
  {
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

  if ( wkbType != wkbNone )
    options = CSLSetNameValue( options, "SPATIAL_INDEX", mCheckBoxCreateSpatialIndex->isChecked() ? "YES" : "NO" );

  OGRLayerH hLayer = OGR_DS_CreateLayer( hDS.get(), tableName.toUtf8().constData(), hSRS, wkbType, options );
  CSLDestroy( options );
  if ( hSRS )
    OSRRelease( hSRS );
  if ( !hLayer )
  {
    QString msg( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
    return false;
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QString fieldName( ( *it )->text( 0 ) );
    QString fieldType( ( *it )->text( 1 ) );
    QString fieldWidth( ( *it )->text( 2 ) );

    OGRFieldType ogrType( OFTString );
    if ( fieldType == QLatin1String( "text" ) )
      ogrType = OFTString;
    else if ( fieldType == QLatin1String( "integer" ) )
      ogrType = OFTInteger;
    else if ( fieldType == QLatin1String( "integer64" ) )
      ogrType = OFTInteger64;
    else if ( fieldType == QLatin1String( "real" ) )
      ogrType = OFTReal;
    else if ( fieldType == QLatin1String( "date" ) )
      ogrType = OFTDate;
    else if ( fieldType == QLatin1String( "datetime" ) )
      ogrType = OFTDateTime;

    int ogrWidth = fieldWidth.toInt();

    gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), ogrType ) );
    OGR_Fld_SetWidth( fld.get(), ogrWidth );

    if ( OGR_L_CreateField( hLayer, fld.get(), true ) != OGRERR_NONE )
    {
      if ( !property( "hideDialogs" ).toBool() )
      {
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ),
                               tr( "Creation of field %1 failed (OGR error: %2)" )
                               .arg( fieldName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      }
      return false;
    }

    ++it;
  }

  // In GDAL >= 2.0, the driver implements a deferred creation strategy, so
  // issue a command that will force table creation
  CPLErrorReset();
  OGR_L_ResetReading( hLayer );
  if ( CPLGetLastErrorType() != CE_None )
  {
    QString msg( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
    return false;
  }
  hDS.reset();

  QString uri( QStringLiteral( "%1|layername=%2" ).arg( fileName, tableName ) );
  QString userVisiblelayerName( layerIdentifier.isEmpty() ? tableName : layerIdentifier );
  QgsVectorLayer *layer = new QgsVectorLayer( uri, userVisiblelayerName, QStringLiteral( "ogr" ) );
  if ( layer->isValid() )
  {
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
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( tableName ) );
    delete layer;
  }

  return false;
}

void QgsNewGeoPackageLayerDialog::setOverwriteBehavior( OverwriteBehavior behavior )
{
  mBehavior = behavior;
}

void QgsNewGeoPackageLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-geopackage-layer" ) );
}
