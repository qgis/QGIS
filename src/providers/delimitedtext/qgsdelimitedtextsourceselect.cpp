/***************************************************************************
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   sherman at mrcc.com                                                   *
 *                                                                         *
 *   GUI for loading a delimited text file as a layer in QGIS              *
 *   This plugin works in conjunction with the delimited text data          *
 *   provider plugin                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "qgsdelimitedtextsourceselect.h"

#include "qgisinterface.h"
#include "qgslogger.h"
#include "qgsvectordataprovider.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgsgui.h"
#include "qgsapplication.h"
#include "qgsvariantutils.h"

#include <QButtonGroup>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>
#include <QTextCodec>
#include <QUrl>
#include <QUrlQuery>

const int MAX_SAMPLE_LENGTH = 200;

QgsDelimitedTextSourceSelect::QgsDelimitedTextSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
  , mFile( std::make_unique<QgsDelimitedTextFile>() )
  , mSettingsKey( QStringLiteral( "/Plugin-DelimitedText" ) )
{

  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDelimitedTextSourceSelect::showHelp );

  bgFileFormat = new QButtonGroup( this );
  bgFileFormat->addButton( delimiterCSV, swFileFormat->indexOf( swpCSVOptions ) );
  bgFileFormat->addButton( delimiterChars, swFileFormat->indexOf( swpDelimOptions ) );
  bgFileFormat->addButton( delimiterRegexp, swFileFormat->indexOf( swpRegexpOptions ) );

  bgGeomType = new QButtonGroup( this );
  bgGeomType->addButton( geomTypeXY, swGeomType->indexOf( swpGeomXY ) );
  bgGeomType->addButton( geomTypeWKT, swGeomType->indexOf( swpGeomWKT ) );
  bgGeomType->addButton( geomTypeNone, swGeomType->indexOf( swpGeomNone ) );

  connect( bgFileFormat, static_cast < void ( QButtonGroup::* )( int ) > ( &QButtonGroup::buttonClicked ), swFileFormat, &QStackedWidget::setCurrentIndex );
  connect( bgGeomType, static_cast < void ( QButtonGroup::* )( int ) > ( &QButtonGroup::buttonClicked ), swGeomType, &QStackedWidget::setCurrentIndex );
  connect( bgGeomType, static_cast < void ( QButtonGroup::* )( int ) > ( &QButtonGroup::buttonClicked ), this, &QgsDelimitedTextSourceSelect::updateCrsWidgetVisibility );

  cmbEncoding->clear();
  cmbEncoding->addItems( QgsVectorDataProvider::availableEncodings() );
  cmbEncoding->setCurrentIndex( cmbEncoding->findText( QStringLiteral( "UTF-8" ) ) );

  loadSettings();
  mBooleanFalse->setEnabled( ! mBooleanTrue->text().isEmpty() );
  updateFieldsAndEnable();

  connect( txtLayerName, &QLineEdit::textChanged, this, &QgsDelimitedTextSourceSelect::enableAccept );
  connect( cmbEncoding, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( delimiterCSV, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( delimiterChars, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( delimiterRegexp, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( cbxDelimComma, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxDelimSpace, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxDelimTab, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxDelimSemicolon, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxDelimColon, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( txtDelimiterOther, &QLineEdit::textChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( txtQuoteChars, &QLineEdit::textChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( txtEscapeChars, &QLineEdit::textChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( txtDelimiterRegexp, &QLineEdit::textChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( rowCounter, static_cast < void ( QSpinBox::* )( int ) > ( &QSpinBox::valueChanged ), this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxUseHeader, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxSkipEmptyFields, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxTrimFields, &QCheckBox::stateChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( cbxPointIsComma, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );
  connect( cbxXyDms, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( crsGeometry, &QgsProjectionSelectionWidget::crsChanged, this, &QgsDelimitedTextSourceSelect::updateFieldsAndEnable );

  connect( mBooleanTrue, &QLineEdit::textChanged, mBooleanFalse, [ = ]
  {
    mBooleanFalse->setEnabled( ! mBooleanTrue->text().isEmpty() );
    updateFieldsAndEnable();
  } );

  connect( mBooleanFalse, &QLineEdit::textChanged, mBooleanFalse, [ = ]
  {
    updateFieldsAndEnable();
  } );

  const QgsSettings settings;
  mFileWidget->setDialogTitle( tr( "Choose a Delimited Text File to Open" ) );
  mFileWidget->setFilter( tr( "Text files" ) + QStringLiteral( " (*.txt *.csv *.dat *.wkt *.tsv);;" ) + tr( "All files" ) + QStringLiteral( " (* *.*)" ) );
  mFileWidget->setSelectedFilter( settings.value( mSettingsKey + QStringLiteral( "/file_filter" ), QString() ).toString() );
  mMaxFields = settings.value( mSettingsKey + QStringLiteral( "/max_fields" ), DEFAULT_MAX_FIELDS ).toInt();
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, &QgsDelimitedTextSourceSelect::updateFileName );

  updateCrsWidgetVisibility();
  mScanWidget->hide( );
}

void QgsDelimitedTextSourceSelect::addButtonClicked()
{
  // The following conditions should not be hit! OK will not be enabled...
  if ( txtLayerName->text().isEmpty() )
  {
    QMessageBox::warning( this, tr( "No layer name" ), tr( "Please enter a layer name before adding the layer to the map" ) );
    txtLayerName->setFocus();
    return;
  }
  if ( delimiterChars->isChecked() )
  {
    if ( selectedChars().isEmpty() )
    {
      QMessageBox::warning( this, tr( "No delimiters set" ), tr( "Use one or more characters as the delimiter, or choose a different delimiter type" ) );
      txtDelimiterOther->setFocus();
      return;
    }
  }
  if ( delimiterRegexp->isChecked() )
  {
    const QRegularExpression re( txtDelimiterRegexp->text() );
    if ( ! re.isValid() )
    {
      QMessageBox::warning( this, tr( "Invalid regular expression" ), tr( "Please enter a valid regular expression as the delimiter, or choose a different delimiter type" ) );
      txtDelimiterRegexp->setFocus();
      return;
    }
  }
  if ( ! mFile->isValid() )
  {
    QMessageBox::warning( this, tr( "Invalid delimited text file" ), tr( "Please enter a valid file and delimiter" ) );
    return;
  }

  cancelScanTask();

  //Build the delimited text URI from the user provided information
  const QString datasourceUrl { url( )};

  // store the settings
  saveSettings();
  saveSettingsForFile( mFileWidget->filePath() );


  // add the layer to the map
  emit addVectorLayer( datasourceUrl, txtLayerName->text() );

  // clear the file and layer name show something has happened, ready for another file

  mFileWidget->setFilePath( QString() );
  txtLayerName->setText( QString() );

  if ( widgetMode() == QgsProviderRegistry::WidgetMode::None )
  {
    accept();
  }
}


QString QgsDelimitedTextSourceSelect::selectedChars()
{
  QString chars;
  if ( cbxDelimComma->isChecked() )
    chars.append( ',' );
  if ( cbxDelimSpace->isChecked() )
    chars.append( ' ' );
  if ( cbxDelimTab->isChecked() )
    chars.append( '\t' );
  if ( cbxDelimSemicolon->isChecked() )
    chars.append( ';' );
  if ( cbxDelimColon->isChecked() )
    chars.append( ':' );
  chars = QgsDelimitedTextFile::encodeChars( chars );
  chars.append( txtDelimiterOther->text() );
  return chars;
}
void QgsDelimitedTextSourceSelect::setSelectedChars( const QString &delimiters )
{
  QString chars = QgsDelimitedTextFile::decodeChars( delimiters );
  cbxDelimComma->setChecked( chars.contains( ',' ) );
  cbxDelimSpace->setChecked( chars.contains( ' ' ) );
  cbxDelimTab->setChecked( chars.contains( '\t' ) );
  cbxDelimColon->setChecked( chars.contains( ':' ) );
  cbxDelimSemicolon->setChecked( chars.contains( ';' ) );
  chars = chars.remove( QRegularExpression( QStringLiteral( "[ ,:;\t]" ) ) );
  chars = QgsDelimitedTextFile::encodeChars( chars );
  txtDelimiterOther->setText( chars );
}

void QgsDelimitedTextSourceSelect::loadSettings( const QString &subkey, bool loadGeomSettings )
{
  const QgsSettings settings;

  // at startup, fetch the last used delimiter and directory from
  // settings
  QString key = mSettingsKey;
  if ( ! subkey.isEmpty() ) key.append( '/' ).append( subkey );

  // and how to use the delimiter
  const QString delimiterType = settings.value( key + "/delimiterType", "" ).toString();
  if ( delimiterType == QLatin1String( "chars" ) )
  {
    delimiterChars->setChecked( true );
  }
  else if ( delimiterType == QLatin1String( "regexp" ) )
  {
    delimiterRegexp->setChecked( true );
  }
  else if ( delimiterType == QLatin1String( "csv" ) )
  {
    delimiterCSV->setChecked( true );
  }
  swFileFormat->setCurrentIndex( bgFileFormat->checkedId() );

  const QString encoding = settings.value( key + "/encoding", "" ).toString();
  if ( ! encoding.isEmpty() ) cmbEncoding->setCurrentIndex( cmbEncoding->findText( encoding ) );
  const QString delimiters = settings.value( key + "/delimiters", "" ).toString();
  if ( ! delimiters.isEmpty() ) setSelectedChars( delimiters );

  txtQuoteChars->setText( settings.value( key + "/quoteChars", "\"" ).toString() );
  txtEscapeChars->setText( settings.value( key + "/escapeChars", "\"" ).toString() );

  const QString regexp = settings.value( key + "/delimiterRegexp", "" ).toString();
  if ( ! regexp.isEmpty() ) txtDelimiterRegexp->setText( regexp );

  rowCounter->setValue( settings.value( key + "/startFrom", 0 ).toInt() );
  cbxUseHeader->setChecked( settings.value( key + "/useHeader", "true" ) != "false" );
  cbxDetectTypes->setChecked( settings.value( key + "/detectTypes", "true" ) != "false" );
  cbxTrimFields->setChecked( settings.value( key + "/trimFields", "false" ) == "true" );
  cbxSkipEmptyFields->setChecked( settings.value( key + "/skipEmptyFields", "false" ) == "true" );
  cbxPointIsComma->setChecked( settings.value( key + "/decimalPoint", "." ).toString().contains( ',' ) );
  cbxSubsetIndex->setChecked( settings.value( key + "/subsetIndex", "false" ) == "true" );
  cbxSpatialIndex->setChecked( settings.value( key + "/spatialIndex", "false" ) == "true" );
  cbxWatchFile->setChecked( settings.value( key + "/watchFile", "false" ) == "true" );
  mBooleanFalse->setText( settings.value( key + "/booleanFalse", "" ).toString() );
  mBooleanTrue->setText( settings.value( key + "/booleanTrue", "" ).toString() );

  if ( loadGeomSettings )
  {
    const QString geomColumnType = settings.value( key + "/geomColumnType", "xy" ).toString();
    if ( geomColumnType == QLatin1String( "xy" ) ) geomTypeXY->setChecked( true );
    else if ( geomColumnType == QLatin1String( "wkt" ) ) geomTypeWKT->setChecked( true );
    else geomTypeNone->setChecked( true );
    cbxXyDms->setChecked( settings.value( key + "/xyDms", "false" ) == "true" );
    swGeomType->setCurrentIndex( bgGeomType->checkedId() );
    const QString authid = settings.value( key + "/crs", "" ).toString();
    const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( authid );
    if ( crs.isValid() )
    {
      crsGeometry->setCrs( crs );
    }
  }

}

void QgsDelimitedTextSourceSelect::saveSettings( const QString &subkey, bool saveGeomSettings )
{
  QgsSettings settings;
  QString key = mSettingsKey;
  if ( ! subkey.isEmpty() ) key.append( '/' ).append( subkey );
  settings.setValue( key + "/encoding", cmbEncoding->currentText() );
  settings.setValue( key + "/geometry", saveGeometry() );

  if ( delimiterCSV->isChecked() )
    settings.setValue( key + "/delimiterType", "csv" );
  else if ( delimiterChars->isChecked() )
    settings.setValue( key + "/delimiterType", "chars" );
  else
    settings.setValue( key + "/delimiterType", "regexp" );
  settings.setValue( key + "/delimiters", selectedChars() );
  settings.setValue( key + "/quoteChars", txtQuoteChars->text() );
  settings.setValue( key + "/escapeChars", txtEscapeChars->text() );
  settings.setValue( key + "/delimiterRegexp", txtDelimiterRegexp->text() );
  settings.setValue( key + "/startFrom", rowCounter->value() );
  settings.setValue( key + "/useHeader", cbxUseHeader->isChecked() ? "true" : "false" );
  settings.setValue( key + "/detectTypes", cbxDetectTypes->isChecked() ? "true" : "false" );
  settings.setValue( key + "/trimFields", cbxTrimFields->isChecked() ? "true" : "false" );
  settings.setValue( key + "/skipEmptyFields", cbxSkipEmptyFields->isChecked() ? "true" : "false" );
  settings.setValue( key + "/decimalPoint", cbxPointIsComma->isChecked() ? "," : "." );
  settings.setValue( key + "/subsetIndex", cbxSubsetIndex->isChecked() ? "true" : "false" );
  settings.setValue( key + "/spatialIndex", cbxSpatialIndex->isChecked() ? "true" : "false" );
  settings.setValue( key + "/watchFile", cbxWatchFile->isChecked() ? "true" : "false" );
  settings.setValue( key + "/booleanFalse", mBooleanFalse->text() );
  settings.setValue( key + "/booleanTrue", mBooleanTrue->text() );
  if ( saveGeomSettings )
  {
    QString geomColumnType = QStringLiteral( "none" );
    if ( geomTypeXY->isChecked() ) geomColumnType = QStringLiteral( "xy" );
    if ( geomTypeWKT->isChecked() ) geomColumnType = QStringLiteral( "wkt" );
    settings.setValue( key + "/geomColumnType", geomColumnType );
    settings.setValue( key + "/xyDms", cbxXyDms->isChecked() ? "true" : "false" );
    if ( crsGeometry->crs().isValid() )
    {
      settings.setValue( key + "/crs", crsGeometry->crs().authid() );
    }
  }

}

void QgsDelimitedTextSourceSelect::loadSettingsForFile( const QString &filename )
{
  if ( filename.isEmpty() ) return;
  const QFileInfo fi( filename );
  const QString filetype = fi.suffix();
  // Don't expect to change settings if not changing file type
  if ( filetype != mLastFileType ) loadSettings( fi.suffix(), true );
  mLastFileType = filetype;
}

void QgsDelimitedTextSourceSelect::saveSettingsForFile( const QString &filename )
{
  if ( filename.isEmpty() ) return;
  const QFileInfo fi( filename );
  saveSettings( fi.suffix(), true );
}


bool QgsDelimitedTextSourceSelect::loadDelimitedFileDefinition()
{
  mFile->setFileName( mFileWidget->filePath() );
  mFile->setEncoding( cmbEncoding->currentText() );
  if ( delimiterChars->isChecked() )
  {
    mFile->setTypeCSV( selectedChars(), txtQuoteChars->text(), txtEscapeChars->text() );
  }
  else if ( delimiterRegexp->isChecked() )
  {
    mFile->setTypeRegexp( txtDelimiterRegexp->text() );
  }
  else
  {
    mFile->setTypeCSV();
  }
  mFile->setSkipLines( rowCounter->value() );
  mFile->setUseHeader( cbxUseHeader->isChecked() );
  mFile->setDiscardEmptyFields( cbxSkipEmptyFields->isChecked() );
  mFile->setTrimFields( cbxTrimFields->isChecked() );
  mFile->setMaxFields( mMaxFields );
  return mFile->isValid();
}


void QgsDelimitedTextSourceSelect::updateFieldLists()
{
  // Update the x and y field drop-down boxes
  QgsDebugMsgLevel( QStringLiteral( "Updating field lists" ), 3 );

  disconnect( cmbXField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
  disconnect( cmbYField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
  disconnect( cmbWktField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
  disconnect( geomTypeXY, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );
  disconnect( geomTypeWKT, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );
  disconnect( geomTypeNone, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );

  const QString columnX = cmbXField->currentText();
  const QString columnY = cmbYField->currentText();
  const QString columnZ = cmbZField->currentText();
  const QString columnM = cmbMField->currentText();
  const QString columnWkt = cmbWktField->currentText();

  // clear the field lists
  cmbXField->clear();
  cmbYField->clear();
  cmbZField->clear();
  cmbMField->clear();
  cmbWktField->clear();

  // clear the sample text box
  tblSample->clear();
  tblSample->setColumnCount( 0 );
  tblSample->setRowCount( 0 );

  if ( ! loadDelimitedFileDefinition() )
    return;

  // Put a sample set of records into the sample box.  Also while scanning assess suitability of
  // fields for use as coordinate and WKT fields


  QList<bool> isValidCoordinate;
  QList<bool> isValidWkt;
  QList<bool> isEmpty;
  int counter = 0;
  mBadRowCount = 0;
  QStringList values;
  const QRegularExpression wktre( "^\\s*(?:MULTI)?(?:POINT|LINESTRING|POLYGON)\\s*Z?\\s*M?\\(", QRegularExpression::CaseInsensitiveOption );

  while ( counter < mExampleRowCount )
  {
    const QgsDelimitedTextFile::Status status = mFile->nextRecord( values );
    if ( status == QgsDelimitedTextFile::RecordEOF ) break;
    if ( status != QgsDelimitedTextFile::RecordOk ) { mBadRowCount++; continue; }
    counter++;


    // Look at count of non-blank fields

    int nv = values.size();
    while ( nv > 0 && values[nv - 1].isEmpty() ) nv--;

    if ( isEmpty.size() < nv )
    {
      while ( isEmpty.size() < nv )
      {
        isEmpty.append( true );
        isValidCoordinate.append( false );
        isValidWkt.append( false );
      }
      tblSample->setColumnCount( nv );
    }

    tblSample->setRowCount( counter );

    const bool xyDms = cbxXyDms->isChecked();

    for ( int i = 0; i < tblSample->columnCount(); i++ )
    {
      QString value = i < nv ? values[i] : QString();
      if ( value.length() > MAX_SAMPLE_LENGTH )
        value = value.mid( 0, MAX_SAMPLE_LENGTH ) + QChar( 0x2026 );
      QTableWidgetItem *item = new QTableWidgetItem( value );
      tblSample->setItem( counter - 1, i, item );
      if ( ! value.isEmpty() )
      {
        if ( isEmpty[i] )
        {
          isEmpty[i] = false;
          isValidCoordinate[i] = true;
          isValidWkt[i] = true;
        }
        if ( isValidCoordinate[i] )
        {
          bool ok = true;
          if ( cbxPointIsComma->isChecked() )
          {
            value.replace( ',', '.' );
          }
          if ( xyDms )
          {
            const QRegularExpressionMatch match = QgsDelimitedTextProvider::sCrdDmsRegexp.match( value );
            ok = match.capturedStart() == 0;
          }
          else
          {
            ( void )value.toDouble( &ok );
          }
          isValidCoordinate[i] = ok;
        }
        if ( isValidWkt[i] )
        {
          value.remove( QgsDelimitedTextProvider::sWktPrefixRegexp );
          isValidWkt[i] = value.contains( wktre );
        }
      }
    }
  }


  QStringList fieldList = mFile->fieldNames();

  if ( isEmpty.size() < fieldList.size() )
  {
    while ( isEmpty.size() < fieldList.size() )
    {
      isEmpty.append( true );
      isValidCoordinate.append( false );
      isValidWkt.append( false );
    }
    tblSample->setColumnCount( fieldList.size() );
  }

  tblSample->insertRow( 0 );
  QStringList verticalHeaderLabels;
  verticalHeaderLabels.push_back( QString( ) );

  for ( int i = 1; i <= tblSample->rowCount(); i++ )
  {
    verticalHeaderLabels.push_back( QString::number( i ) );
  }

  tblSample->setVerticalHeaderLabels( verticalHeaderLabels );


  for ( int column = 0; column < tblSample->columnCount(); column++ )
  {
    QComboBox *typeCombo = new QComboBox( tblSample );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::String ), QgsVariantUtils::typeToDisplayString( QVariant::String ), "text" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::Int ), QgsVariantUtils::typeToDisplayString( QVariant::Int ), "integer" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::LongLong ), QgsVariantUtils::typeToDisplayString( QVariant::LongLong ), "longlong" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::Double ), "double" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::Bool ), QgsVariantUtils::typeToDisplayString( QVariant::Bool ), "bool" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::Date ), QgsVariantUtils::typeToDisplayString( QVariant::Date ), "date" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::Time ), QgsVariantUtils::typeToDisplayString( QVariant::Time ), "time" );
    typeCombo->addItem( QgsFields::iconForFieldType( QVariant::DateTime ), QgsVariantUtils::typeToDisplayString( QVariant::DateTime ), "datetime" );
    connect( typeCombo, qOverload<int>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
    {
      mOverriddenFields.insert( column );
    } );
    tblSample->setCellWidget( 0, column, typeCombo );
  }

  tblSample->setHorizontalHeaderLabels( fieldList );
  tblSample->resizeColumnsToContents();
  tblSample->resizeRowsToContents();

  // Run the scan in a separate thread
  cancelScanTask();

  mScanTask = new QgsDelimitedTextFileScanTask( url( /* skip overridden types */ true ) );
  mCancelButton->show();
  connect( mScanTask, &QgsDelimitedTextFileScanTask::scanCompleted, this, [ = ]( const QgsFields & fields )
  {
    updateFieldTypes( fields );
    mScanWidget->hide( );
  } );

  connect( mScanTask, &QgsDelimitedTextFileScanTask::scanStarted, this, [ = ]( const QgsFields & fields )
  {
    updateFieldTypes( fields );
  } );

  connect( mCancelButton, &QPushButton::clicked, this, &QgsDelimitedTextSourceSelect::cancelScanTask );

  connect( mScanTask, &QgsDelimitedTextFileScanTask::processedCountChanged, this, [ = ]( unsigned long long recordsScanned )
  {
    mScanWidget->show();
    mProgressLabel->setText( tr( "Column types detection in progress: %L1 records read" ).arg( static_cast<unsigned long long>( recordsScanned ) ) );
  } );

  // This is required because QgsTask emits a progress changed 100 when done
  connect( mScanTask, &QgsDelimitedTextFileScanTask::taskCompleted, this, [ = ]
  {
    mScanWidget->hide( );
  } );

  QgsApplication::taskManager()->addTask( mScanTask, 100 );

  // We don't know anything about a text based field other
  // than its name. All fields are assumed to be text
  // As we ignore blank fields we need to map original index
  // of selected fields to index in combo box.

  // Add an empty item for M and Z field
  cmbMField->addItem( QString() );
  cmbZField->addItem( QString() );

  int fieldNo = 0;
  for ( int i = 0; i < fieldList.size(); i++ )
  {
    const QString field = fieldList[i];
    // skip empty field names
    if ( field.isEmpty() ) continue;
    cmbXField->addItem( field );
    cmbYField->addItem( field );
    cmbZField->addItem( field );
    cmbMField->addItem( field );
    cmbWktField->addItem( field );
    fieldNo++;
  }

  // Try resetting current values for column names

  cmbWktField->setCurrentIndex( cmbWktField->findText( columnWkt ) );
  cmbXField->setCurrentIndex( cmbXField->findText( columnX ) );
  cmbYField->setCurrentIndex( cmbYField->findText( columnY ) );
  cmbZField->setCurrentIndex( cmbYField->findText( columnZ ) );
  cmbMField->setCurrentIndex( cmbYField->findText( columnM ) );

  // Now try setting optional X,Y fields - will only reset the fields if
  // not already set.

  trySetXYField( fieldList, isValidCoordinate, QStringLiteral( "longitude" ), QStringLiteral( "latitude" ) );
  trySetXYField( fieldList, isValidCoordinate, QStringLiteral( "lon" ), QStringLiteral( "lat" ) );
  trySetXYField( fieldList, isValidCoordinate, QStringLiteral( "east" ), QStringLiteral( "north" ) );
  trySetXYField( fieldList, isValidCoordinate, QStringLiteral( "x" ), QStringLiteral( "y" ) );
  trySetXYField( fieldList, isValidCoordinate, QStringLiteral( "e" ), QStringLiteral( "n" ) );

  // And also a WKT field if there is one

  if ( cmbWktField->currentIndex() < 0 )
  {
    for ( int i = 0; i < fieldList.size(); i++ )
    {
      if ( ! isValidWkt[i] ) continue;
      const int index = cmbWktField->findText( fieldList[i] );
      if ( index >= 0 )
      {
        cmbWktField->setCurrentIndex( index );
        break;
      }
    }
  }

  const bool haveFields = fieldNo > 0;

  if ( !geomTypeNone->isChecked() )
  {
    const bool isXY = cmbWktField->currentIndex() < 0 ||
                      ( geomTypeXY->isChecked() &&
                        ( cmbXField->currentIndex() >= 0 && cmbYField->currentIndex() >= 0 ) );
    geomTypeXY->setChecked( isXY );
    geomTypeWKT->setChecked( ! isXY );
  }
  swGeomType->setCurrentIndex( bgGeomType->checkedId() );

  if ( haveFields )
  {
    connect( cmbXField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
    connect( cmbYField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
    connect( cmbWktField, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsDelimitedTextSourceSelect::enableAccept );
    connect( geomTypeXY, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );
    connect( geomTypeWKT, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );
    connect( geomTypeNone, &QAbstractButton::toggled, this, &QgsDelimitedTextSourceSelect::enableAccept );
  }

}

bool QgsDelimitedTextSourceSelect::trySetXYField( QStringList &fields, QList<bool> &isValidNumber, const QString &xname, const QString &yname )
{
  // If fields already set, then nothing to do
  if ( cmbXField->currentIndex() >= 0 && cmbYField->currentIndex() >= 0 ) return true;

  // Try and find a valid field name matching the x field
  int indexX = -1;
  int indexY = -1;

  for ( int i = 0; i < fields.size(); i++ )
  {
    // Only interested in number fields containing the xname string
    // that are in the X combo box
    if ( ! isValidNumber[i] ) continue;
    if ( ! fields[i].contains( xname, Qt::CaseInsensitive ) ) continue;
    indexX = cmbXField->findText( fields[i] );
    if ( indexX < 0 ) continue;

    // Now look for potential y fields, like xname with x replaced with y
    const QString xfield( fields[i] );
    int from = 0;
    while ( true )
    {
      const int pos = xfield.indexOf( xname, from, Qt::CaseInsensitive );
      if ( pos < 0 ) break;
      from = pos + 1;
      const QString yfield = xfield.mid( 0, pos ) + yname + xfield.mid( pos + xname.size() );
      if ( ! fields.contains( yfield, Qt::CaseInsensitive ) ) continue;
      for ( int iy = 0; iy < fields.size(); iy++ )
      {
        if ( ! isValidNumber[iy] ) continue;
        if ( iy == i ) continue;
        if ( fields[iy].compare( yfield, Qt::CaseInsensitive ) == 0 )
        {
          indexY = cmbYField->findText( fields[iy] );
          break;
        }
      }
      if ( indexY >= 0 ) break;
    }
    if ( indexY >= 0 ) break;
  }
  if ( indexY >= 0 )
  {
    cmbXField->setCurrentIndex( indexX );
    cmbYField->setCurrentIndex( indexY );
  }
  return indexY >= 0;
}

void QgsDelimitedTextSourceSelect::updateFileName()
{
  QgsSettings settings;
  settings.setValue( mSettingsKey + "/file_filter", mFileWidget->selectedFilter() );

  // put a default layer name in the text entry
  const QString filename = mFileWidget->filePath();
  const QFileInfo finfo( filename );
  if ( finfo.exists() )
  {
    QgsSettings settings;
    settings.setValue( mSettingsKey + "/text_path", finfo.path() );
  }

  txtLayerName->setText( finfo.completeBaseName() );
  loadSettingsForFile( filename );
  updateFieldsAndEnable();
}

void QgsDelimitedTextSourceSelect::updateFieldsAndEnable()
{
  updateFieldLists();
  enableAccept();
}

bool QgsDelimitedTextSourceSelect::validate()
{
  // Check that input data is valid - provide a status message if not..

  QString message;
  bool enabled = false;

  if ( mFileWidget->filePath().trimmed().isEmpty() )
  {
    message = tr( "Please select an input file" );
  }
  else if ( ! QFileInfo::exists( mFileWidget->filePath() ) )
  {
    message = tr( "File %1 does not exist" ).arg( mFileWidget->filePath() );
  }
  else if ( txtLayerName->text().isEmpty() )
  {
    message = tr( "Please enter a layer name" );
  }
  else if ( delimiterChars->isChecked() && selectedChars().isEmpty() )
  {
    message = tr( "At least one delimiter character must be specified" );
  }

  if ( message.isEmpty() && delimiterRegexp->isChecked() )
  {
    const QRegularExpression re( txtDelimiterRegexp->text() );
    if ( ! re.isValid() )
    {
      message = tr( "Regular expression is not valid" );
    }
    else if ( re.pattern().startsWith( '^' ) && re.captureCount() == 0 )
    {
      message = tr( "^.. expression needs capture groups" );
    }
    lblRegexpError->setText( message );
  }
  if ( ! message.isEmpty() )
  {
    // continue...
  }
  // Hopefully won't hit this none-specific message, but just in case ...
  else if ( ! mFile->isValid() )
  {
    message = tr( "Definition of filename and delimiters is not valid" );
  }
  // Assume that the sample table will have been populated if data was found
  else if ( tblSample->rowCount() == 0 )
  {
    message = tr( "No data found in file" );
    if ( mBadRowCount > 0 )
    {
      message = message + " (" + tr( "%n badly formatted record(s) discarded", nullptr, mBadRowCount ) + ')';
    }
  }
  else if ( geomTypeXY->isChecked() && ( cmbXField->currentText().isEmpty()  || cmbYField->currentText().isEmpty() ) )
  {
    message = tr( "X and Y field names must be selected" );
  }
  else if ( geomTypeXY->isChecked() && ( cmbXField->currentText() == cmbYField->currentText() ) )
  {
    message = tr( "X and Y field names cannot be the same" );
  }
  else if ( geomTypeWKT->isChecked() && cmbWktField->currentText().isEmpty() )
  {
    message = tr( "The WKT field name must be selected" );
  }
  else if ( ! geomTypeNone->isChecked() && ! crsGeometry->crs().isValid() )
  {
    message = tr( "The CRS must be selected" );
  }
  else
  {
    enabled = true;
    if ( mBadRowCount > 0 )
    {
      message = tr( "%n badly formatted record(s) discarded from sample data", nullptr, mBadRowCount );
    }

  }

  if ( mBooleanTrue->text().isEmpty() != mBooleanFalse->text().isEmpty() )
  {
    message = tr( "Custom boolean values for \"true\" or \"false\" is missing." );
  }

  if ( ! message.isEmpty() )
  {
    QgsDebugMsgLevel( QStringLiteral( "Validation error: %1" ).arg( message ), 2 );
  }

  lblStatus->setText( message );
  return enabled;
}

void QgsDelimitedTextSourceSelect::updateFieldTypes( const QgsFields &fields )
{

  mFields = fields;

  for ( int column = 0; column < tblSample->columnCount(); column++ )
  {
    if ( ! mOverriddenFields.contains( column ) )
    {
      const QString fieldName { tblSample->horizontalHeaderItem( column )->text() };
      const int fieldIdx { mFields.lookupField( fieldName ) };
      if ( fieldIdx >= 0 )
      {
        QComboBox *typeCombo { qobject_cast<QComboBox *>( tblSample->cellWidget( 0, column ) ) };
        const QString fieldTypeName { mFields.field( fieldIdx ).typeName() };
        if ( typeCombo && typeCombo->currentData( ) != fieldTypeName && typeCombo->findData( fieldTypeName ) >= 0 )
        {
          QgsDebugMsgLevel( QStringLiteral( "Setting field type %1 from %2 to %3" ).arg( fieldName, typeCombo->currentData().toString(), fieldTypeName ), 2 );
          QgsSignalBlocker( typeCombo )->setCurrentIndex( typeCombo->findData( fieldTypeName ) );
        }
      }
    }
  }
}

void QgsDelimitedTextSourceSelect::enableAccept()
{
  emit enableButtons( validate() );
}

void QgsDelimitedTextSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#importing-a-delimited-text-file" ) );
}

void QgsDelimitedTextSourceSelect::updateCrsWidgetVisibility()
{
  crsGeometry->setVisible( !geomTypeNone->isChecked() );
  textLabelCrs->setVisible( !geomTypeNone->isChecked() );
}

QString QgsDelimitedTextSourceSelect::url( bool skipOverriddenTypes )
{

  QUrl url = mFile->url();
  QUrlQuery query( url );

  query.addQueryItem( QStringLiteral( "detectTypes" ), cbxDetectTypes->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );

  if ( cbxPointIsComma->isChecked() )
  {
    query.addQueryItem( QStringLiteral( "decimalPoint" ), QStringLiteral( "," ) );
  }
  if ( cbxXyDms->isChecked() )
  {
    query.addQueryItem( QStringLiteral( "xyDms" ), QStringLiteral( "yes" ) );
  }

  if ( ! mBooleanFalse->text().isEmpty() && ! mBooleanTrue->text().isEmpty() )
  {
    query.addQueryItem( QStringLiteral( "booleanFalse" ), mBooleanFalse->text() );
    query.addQueryItem( QStringLiteral( "booleanTrue" ), mBooleanTrue->text() );
  }

  bool haveGeom = true;
  if ( geomTypeXY->isChecked() )
  {
    QString field;
    if ( !cmbXField->currentText().isEmpty() && !cmbYField->currentText().isEmpty() )
    {
      field = cmbXField->currentText();
      query.addQueryItem( QStringLiteral( "xField" ), field );
      field = cmbYField->currentText();
      query.addQueryItem( QStringLiteral( "yField" ), field );
    }
    if ( !cmbZField->currentText().isEmpty() )
    {
      field = cmbZField->currentText();
      query.addQueryItem( QStringLiteral( "zField" ), field );
    }
    if ( !cmbMField->currentText().isEmpty() )
    {
      field = cmbMField->currentText();
      query.addQueryItem( QStringLiteral( "mField" ), field );
    }
  }
  else if ( geomTypeWKT->isChecked() )
  {
    if ( ! cmbWktField->currentText().isEmpty() )
    {
      const QString field = cmbWktField->currentText();
      query.addQueryItem( QStringLiteral( "wktField" ), field );
    }
    if ( cmbGeometryType->currentIndex() > 0 )
    {
      query.addQueryItem( QStringLiteral( "geomType" ), cmbGeometryType->currentText() );
    }
  }
  else
  {
    haveGeom = false;
    query.addQueryItem( QStringLiteral( "geomType" ), QStringLiteral( "none" ) );
  }
  if ( haveGeom )
  {
    const QgsCoordinateReferenceSystem crs = crsGeometry->crs();
    if ( crs.isValid() )
    {
      query.addQueryItem( QStringLiteral( "crs" ), crs.authid() );
    }

  }

  if ( ! geomTypeNone->isChecked() )
  {
    query.addQueryItem( QStringLiteral( "spatialIndex" ), cbxSpatialIndex->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );
  }

  query.addQueryItem( QStringLiteral( "subsetIndex" ), cbxSubsetIndex->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );
  query.addQueryItem( QStringLiteral( "watchFile" ), cbxWatchFile->isChecked() ? QStringLiteral( "yes" ) : QStringLiteral( "no" ) );

  if ( ! skipOverriddenTypes )
  {
    // Set field types if overridden
    for ( int column = 0; column < tblSample->columnCount(); column++ )
    {
      const QString fieldName { tblSample->horizontalHeaderItem( column )->text() };
      const int fieldIdx { mFields.lookupField( fieldName ) };
      if ( fieldIdx >= 0 )
      {
        QComboBox *typeCombo { qobject_cast<QComboBox *>( tblSample->cellWidget( 0, column ) ) };
        const QString fieldTypeName { mFields.field( fieldName ).typeName() };
        if ( typeCombo && typeCombo->currentData().toString() != fieldTypeName )
        {
          QgsDebugMsgLevel( QStringLiteral( "Overriding field %1 from %2 to %3" ).arg( fieldName, fieldTypeName, typeCombo->currentData().toString() ), 2 );
          query.addQueryItem( QStringLiteral( "field" ),
                              QString( fieldName ).replace( ':', QLatin1String( "%3A" ) ) + ':' +  typeCombo->currentData().toString() );
        }
      }
    }
  }

  url.setQuery( query );
  return QString::fromLatin1( url.toEncoded() );
}

void QgsDelimitedTextSourceSelect::cancelScanTask()
{
  // This will cancel the existing task (if any)
  if ( mScanTask )
  {
    mScanTask->cancel();
    mScanTask = nullptr;
  }
}

bool QgsDelimitedTextFileScanTask::run()
{
  QgsDelimitedTextProvider provider(
    mDataSource,
    QgsDataProvider::ProviderOptions(),
    QgsDataProvider::ReadFlag::SkipFeatureCount | QgsDataProvider::ReadFlag::SkipGetExtent | QgsDataProvider::ReadFlag::SkipFullScan );

  connect( &mFeedback, &QgsFeedback::processedCountChanged, this, &QgsDelimitedTextFileScanTask::processedCountChanged );

  if ( provider.isValid() )
  {
    emit scanStarted( provider.fields() );
    provider.scanFile( false, true, &mFeedback );
    emit scanCompleted( provider.fields() );
  }
  else
  {
    emit scanCompleted( QgsFields() );
  }
  return true;
}

void QgsDelimitedTextFileScanTask::cancel()
{
  mFeedback.cancel();
  QgsTask::cancel();
}
