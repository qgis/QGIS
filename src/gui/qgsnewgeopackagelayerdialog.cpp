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

#include <cpl_error.h>
#include <cpl_string.h>
#include <gdal_version.h>
#include <ogr_api.h>
#include <ogr_srs_api.h>

#include "qgis.h"
#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgui.h"
#include "qgsguiutils.h"
#include "qgshelp.h"
#include "qgsiconutils.h"
#include "qgsogrutils.h"
#include "qgsproject.h"
#include "qgsproviderconnectionmodel.h"
#include "qgssettings.h"
#include "qgsvariantutils.h"
#include "qgsvectorlayer.h"

#include <QCompleter>
#include <QFileDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>

#include "moc_qgsnewgeopackagelayerdialog.cpp"

#define DEFAULT_OGR_FID_COLUMN_TITLE "fid" // default value from OGR

QgsNewGeoPackageLayerDialog::QgsNewGeoPackageLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  setObjectName( u"QgsNewGeoPackageLayerDialog"_s );
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
  connect( mButtonUp, &QToolButton::clicked, this, &QgsNewGeoPackageLayerDialog::moveFieldsUp );
  connect( mButtonDown, &QToolButton::clicked, this, &QgsNewGeoPackageLayerDialog::moveFieldsDown );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( u"/mActionNewAttribute.svg"_s ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( u"/mActionDeleteAttribute.svg"_s ) );

  const auto addGeomItem = [this]( OGRwkbGeometryType ogrGeomType ) {
    const Qgis::WkbType qgsType = QgsOgrUtils::ogrGeometryTypeToQgsWkbType( ogrGeomType );
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( qgsType ), QgsWkbTypes::translatedDisplayString( qgsType ), ogrGeomType );
  };

  addGeomItem( wkbNone );
  addGeomItem( wkbPoint );
  addGeomItem( wkbLineString );
  addGeomItem( wkbPolygon );
  addGeomItem( wkbMultiPoint );
  addGeomItem( wkbMultiLineString );
  addGeomItem( wkbMultiPolygon );

#if 0
  // QGIS always create CompoundCurve and there's no real interest of having just CircularString. CompoundCurve are more useful
  addGeomItem( wkbCircularString );
#endif
  addGeomItem( wkbCompoundCurve );
  addGeomItem( wkbCurvePolygon );
  addGeomItem( wkbMultiCurve );
  addGeomItem( wkbMultiSurface );
  addGeomItem( wkbPolyhedralSurface );
  addGeomItem( wkbTIN );
  mGeometryTypeBox->setCurrentIndex( -1 );

  mGeometryWithZCheckBox->setEnabled( false );
  mGeometryWithMCheckBox->setEnabled( false );
  mGeometryColumnEdit->setEnabled( false );
  mGeometryColumnEdit->setText( u"geometry"_s );
  mFeatureIdColumnEdit->setPlaceholderText( QStringLiteral( DEFAULT_OGR_FID_COLUMN_TITLE ) );
  mCheckBoxCreateSpatialIndex->setEnabled( false );
  mCrsSelector->setEnabled( false );
  mCrsSelector->setShowAccuracyWarnings( true );

  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QString ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QString ), "text" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Int ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Int ), "integer" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::LongLong ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::LongLong ), "integer64" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Double ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Double ), "real" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDate ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDate ), "date" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QDateTime ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QDateTime ), "datetime" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::Bool ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::Bool ), "bool" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QByteArray ), QgsVariantUtils::typeToDisplayString( QMetaType::Type::QByteArray ), "binary" );
  mFieldTypeBox->addItem( QgsFields::iconForFieldType( QMetaType::Type::QVariantMap ), tr( "JSON" ), "json" );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  connect( mFieldNameEdit, &QLineEdit::textChanged, this, &QgsNewGeoPackageLayerDialog::fieldNameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewGeoPackageLayerDialog::selectionChanged );
  connect( mTableNameEdit, &QLineEdit::textChanged, this, &QgsNewGeoPackageLayerDialog::checkOk );
  connect( mGeometryTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewGeoPackageLayerDialog::checkOk );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );
  mButtonUp->setEnabled( false );
  mButtonDown->setEnabled( false );

  mCheckBoxCreateSpatialIndex->setChecked( true );

  const QgsSettings settings;
  mFileName->setStorageMode( QgsFileWidget::SaveFile );
  mFileName->setFilter( tr( "GeoPackage" ) + " (*.gpkg)" );
  mFileName->setDialogTitle( tr( "Select Existing or Create a New GeoPackage Database File…" ) );
  mFileName->setDefaultRoot( settings.value( u"UI/lastVectorFileFilterDir"_s, QDir::homePath() ).toString() );
  mFileName->setConfirmOverwrite( false );
  connect( mFileName, &QgsFileWidget::fileChanged, this, [this]( const QString &filePath ) {
    QgsSettings settings;
    const QFileInfo tmplFileInfo( filePath );
    settings.setValue( u"UI/lastVectorFileFilterDir"_s, tmplFileInfo.absolutePath() );
    if ( !filePath.isEmpty() && !mTableNameEdited )
    {
      const QFileInfo fileInfo( filePath );
      mTableNameEdit->setText( fileInfo.baseName() );
    }
    checkOk();
  } );

  QgsProviderConnectionModel *ogrProviderModel = new QgsProviderConnectionModel( u"ogr"_s, this );

  QCompleter *completer = new QCompleter( this );
  completer->setModel( ogrProviderModel );
  completer->setCompletionRole( static_cast<int>( QgsProviderConnectionModel::CustomRole::Uri ) );
  completer->setCompletionMode( QCompleter::PopupCompletion );
  completer->setFilterMode( Qt::MatchContains );
  mFileName->lineEdit()->setCompleter( completer );
}

void QgsNewGeoPackageLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsSelector->setCrs( crs );
}

void QgsNewGeoPackageLayerDialog::lockDatabasePath()
{
  mFileName->setReadOnly( true );
}

void QgsNewGeoPackageLayerDialog::mFieldTypeBox_currentIndexChanged( int )
{
  const QString myType = mFieldTypeBox->currentData( Qt::UserRole ).toString();
  mFieldLengthEdit->setEnabled( myType == "text"_L1 );
  if ( myType != "text"_L1 )
    mFieldLengthEdit->clear();
}


void QgsNewGeoPackageLayerDialog::mGeometryTypeBox_currentIndexChanged( int )
{
  const OGRwkbGeometryType geomType = static_cast<OGRwkbGeometryType>( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );
  const bool isSpatial = geomType != wkbNone;
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
  const bool ok = !mFileName->filePath().isEmpty() && !mTableNameEdit->text().isEmpty() && mGeometryTypeBox->currentIndex() != -1;

  mOkButton->setEnabled( ok );
}

void QgsNewGeoPackageLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mFieldNameEdit->text().isEmpty() )
  {
    const QString myName = mFieldNameEdit->text();
    const QString featureId = mFeatureIdColumnEdit->text().isEmpty() ? QStringLiteral( DEFAULT_OGR_FID_COLUMN_TITLE ) : mFeatureIdColumnEdit->text();
    if ( myName.compare( featureId, Qt::CaseInsensitive ) == 0 )
    {
      QMessageBox::critical( this, tr( "Add Field" ), tr( "The field cannot have the same name as the feature identifier." ) );
      return;
    }

    //use userrole to avoid translated type string
    const QString myType = mFieldTypeBox->currentData( Qt::UserRole ).toString();
    const QString length = mFieldLengthEdit->text();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << length ) );

    checkOk();

    mFieldNameEdit->clear();

    if ( !mFieldNameEdit->hasFocus() )
    {
      mFieldNameEdit->setFocus();
    }
  }
}

void QgsNewGeoPackageLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewGeoPackageLayerDialog::fieldNameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || !mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewGeoPackageLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonUp->setDisabled( mAttributeView->selectedItems().isEmpty() );
  mButtonDown->setDisabled( mAttributeView->selectedItems().isEmpty() );
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

void QgsNewGeoPackageLayerDialog::moveFieldsUp()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == 0 )
    return;

  mAttributeView->insertTopLevelItem( currentRow - 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow - 1, 0 ) );
}

void QgsNewGeoPackageLayerDialog::moveFieldsDown()
{
  int currentRow = mAttributeView->currentIndex().row();
  if ( currentRow == mAttributeView->topLevelItemCount() - 1 )
    return;

  mAttributeView->insertTopLevelItem( currentRow + 1, mAttributeView->takeTopLevelItem( currentRow ) );
  mAttributeView->setCurrentIndex( mAttributeView->model()->index( currentRow + 1, 0 ) );
}

bool QgsNewGeoPackageLayerDialog::apply()
{
  if ( !mFieldNameEdit->text().trimmed().isEmpty() )
  {
    const QString currentFieldName = mFieldNameEdit->text();
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

  QString fileName( mFileName->filePath() );
  if ( !fileName.endsWith( ".gpkg"_L1, Qt::CaseInsensitive ) )
    fileName += ".gpkg"_L1;

  bool createNewDb = false;

  if ( QFile::exists( fileName ) )
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
        QPushButton *addNewLayerButton = msgBox.addButton( tr( "Add New Layer" ), QMessageBox::ActionRole );
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
          const int ret = msgBox.exec();
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
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), tr( "Layer creation failed. GeoPackage driver not found." ) );
    return false;
  }

  gdal::ogr_datasource_unique_ptr hDS;
  if ( createNewDb )
  {
    hDS.reset( OGR_Dr_CreateDataSource( hGpkgDriver, fileName.toUtf8().constData(), nullptr ) );
    if ( !hDS )
    {
      const QString msg( tr( "Creation of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
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
      const QString msg( tr( "Opening of database failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
      return false;
    }
    if ( hDriver != hGpkgDriver )
    {
      const QString msg( tr( "Opening of file succeeded, but this is not a GeoPackage database." ) );
      if ( !property( "hideDialogs" ).toBool() )
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
      return false;
    }
  }

  const QString tableName( mTableNameEdit->text() );

  bool overwriteTable = false;
  if ( OGR_DS_GetLayerByName( hDS.get(), tableName.toUtf8().constData() ) )
  {
    if ( property( "hideDialogs" ).toBool() )
    {
      overwriteTable = property( "question_existing_layer_answer_overwrite" ).toBool();
    }
    else if ( QMessageBox::question( this, tr( "New GeoPackage Layer" ), tr( "A table with the same name already exists. Do you want to overwrite it?" ), QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes )
    {
      overwriteTable = true;
    }

    if ( !overwriteTable )
    {
      return false;
    }
  }

  const QString layerIdentifier( mLayerIdentifierEdit->text() );
  const QString layerDescription( mLayerDescriptionEdit->text() );

  OGRwkbGeometryType wkbType = static_cast<OGRwkbGeometryType>( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  // z-coordinate & m-value.
  if ( mGeometryWithZCheckBox->isChecked() )
    wkbType = OGR_GT_SetZ( wkbType );

  if ( mGeometryWithMCheckBox->isChecked() )
    wkbType = OGR_GT_SetM( wkbType );

  // Check for non-standard GeoPackage geometry types
  const Qgis::WkbType qgisWkbType = static_cast<Qgis::WkbType>( wkbType );
  bool isNonStandardGeomType = false;
  if ( !QgsGuiUtils::warnAboutNonStandardGeoPackageGeometryType( qgisWkbType, this, tr( "New GeoPackage Layer" ), !property( "hideDialogs" ).toBool(), &isNonStandardGeomType ) )
  {
    return false;
  }

  OGRSpatialReferenceH hSRS = nullptr;
  // consider spatial reference system of the layer
  const QgsCoordinateReferenceSystem srs = mCrsSelector->crs();
  if ( wkbType != wkbNone && srs.isValid() )
  {
    hSRS = QgsOgrUtils::crsToOGRSpatialReference( srs );
  }

  // Set options
  char **options = nullptr;

  if ( overwriteTable )
    options = CSLSetNameValue( options, "OVERWRITE", "YES" );
  if ( !layerIdentifier.isEmpty() )
    options = CSLSetNameValue( options, "IDENTIFIER", layerIdentifier.toUtf8().constData() );
  if ( !layerDescription.isEmpty() )
    options = CSLSetNameValue( options, "DESCRIPTION", layerDescription.toUtf8().constData() );

  const QString featureId( mFeatureIdColumnEdit->text() );
  if ( !featureId.isEmpty() )
    options = CSLSetNameValue( options, "FID", featureId.toUtf8().constData() );

  const QString geometryColumn( mGeometryColumnEdit->text() );
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
    const QString msg( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
    return false;
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    const QString fieldName( ( *it )->text( 0 ) );
    const QString fieldType( ( *it )->text( 1 ) );
    const QString fieldWidth( ( *it )->text( 2 ) );

    OGRFieldType ogrType( OFTString );
    OGRFieldSubType ogrSubType = OFSTNone;
    if ( fieldType == "text"_L1 )
      ogrType = OFTString;
    else if ( fieldType == "integer"_L1 )
      ogrType = OFTInteger;
    else if ( fieldType == "integer64"_L1 )
      ogrType = OFTInteger64;
    else if ( fieldType == "real"_L1 )
      ogrType = OFTReal;
    else if ( fieldType == "date"_L1 )
      ogrType = OFTDate;
    else if ( fieldType == "datetime"_L1 )
      ogrType = OFTDateTime;
    else if ( fieldType == "bool"_L1 )
    {
      ogrType = OFTInteger;
      ogrSubType = OFSTBoolean;
    }
    else if ( fieldType == "binary"_L1 )
      ogrType = OFTBinary;
    else if ( fieldType == "json"_L1 )
    {
      ogrType = OFTString;
      ogrSubType = OFSTJSON;
    }

    const int ogrWidth = fieldWidth.toInt();

    const gdal::ogr_field_def_unique_ptr fld( OGR_Fld_Create( fieldName.toUtf8().constData(), ogrType ) );
    if ( ogrSubType != OFSTNone )
      OGR_Fld_SetSubType( fld.get(), ogrSubType );

    if ( ogrType != OFTBinary )
      OGR_Fld_SetWidth( fld.get(), ogrWidth );

    if ( OGR_L_CreateField( hLayer, fld.get(), true ) != OGRERR_NONE )
    {
      if ( !property( "hideDialogs" ).toBool() )
      {
        QMessageBox::critical( this, tr( "New GeoPackage Layer" ), tr( "Creation of field %1 failed (OGR error: %2)" ).arg( fieldName, QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
      }
      return false;
    }

    ++it;
  }

  // In GDAL >= 2.0, the driver implements a deferred creation strategy, so
  // issue a command that will force table creation
  CPLErrorReset();
  OGR_L_ResetReading( hLayer );
  const CPLErr errorType = CPLGetLastErrorType();
  if ( errorType == CE_Failure || errorType == CE_Fatal )
  {
    const QString msg( tr( "Creation of layer failed (OGR error: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), msg );
    return false;
  }
  else if ( errorType == CE_Warning && !isNonStandardGeomType )
  {
    // Show OGR warning only if user was not already warned about non-standard geometry types
    const QString msg( tr( "Layer created with warning (OGR warning: %1)" ).arg( QString::fromUtf8( CPLGetLastErrorMsg() ) ) );
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::warning( this, tr( "New GeoPackage Layer" ), msg );
  }
  hDS.reset();

  const QString uri( u"%1|layername=%2"_s.arg( fileName, tableName ) );
  const QString userVisiblelayerName( layerIdentifier.isEmpty() ? tableName : layerIdentifier );
  const QgsVectorLayer::LayerOptions layerOptions { QgsProject::instance()->transformContext() };
  auto layer = std::make_unique<QgsVectorLayer>( uri, userVisiblelayerName, u"ogr"_s, layerOptions );
  if ( layer->isValid() )
  {
    if ( mAddToProject )
    {
      // register this layer with the central layers registry
      QList<QgsMapLayer *> myList;
      myList << layer.release();
      //addMapLayers returns a list of all successfully added layers
      //so we compare that to our original list.
      if ( myList == QgsProject::instance()->addMapLayers( myList ) )
        return true;
    }
    else
    {
      return true;
    }
  }
  else
  {
    if ( !property( "hideDialogs" ).toBool() )
      QMessageBox::critical( this, tr( "New GeoPackage Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( tableName ) );
  }

  return false;
}

void QgsNewGeoPackageLayerDialog::setOverwriteBehavior( OverwriteBehavior behavior )
{
  mBehavior = behavior;
}

void QgsNewGeoPackageLayerDialog::setAddToProject( bool addToProject )
{
  mAddToProject = addToProject;
}

void QgsNewGeoPackageLayerDialog::showHelp()
{
  QgsHelp::openHelp( u"managing_data_source/create_layers.html#creating-a-new-geopackage-layer"_s );
}
