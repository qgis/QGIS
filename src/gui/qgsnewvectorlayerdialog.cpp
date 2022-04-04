/***************************************************************************
                         qgsnewvectorlayerdialog.cpp  -  description
                             -------------------
    begin                : October 2004
    copyright            : (C) 2004 by Marco Hugentobler
    email                : marco.hugentobler@autoform.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnewvectorlayerdialog.h"
#include "qgsapplication.h"
#include "qgsfilewidget.h"
#include "qgis.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgssettings.h"
#include "qgsogrprovider.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgsfileutils.h"
#include "qgsvariantutils.h"

#include <QPushButton>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>

QgsNewVectorLayerDialog::QgsNewVectorLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewVectorLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewVectorLayerDialog::mRemoveAttributeButton_clicked );
  connect( mFileFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewVectorLayerDialog::mFileFormatComboBox_currentIndexChanged );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewVectorLayerDialog::mTypeBox_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewVectorLayerDialog::showHelp );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );

  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::String ), QgsVariantUtils::typeToDisplayString( QVariant::String ), "String" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Int ), QgsVariantUtils::typeToDisplayString( QVariant::Int ), "Integer" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Double ), QgsVariantUtils::typeToDisplayString( QVariant::Double ), "Real" );
  mTypeBox->addItem( QgsFields::iconForFieldType( QVariant::Date ), QgsVariantUtils::typeToDisplayString( QVariant::Date ), "Date" );

  mWidth->setValidator( new QIntValidator( 1, 255, this ) );
  mPrecision->setValidator( new QIntValidator( 0, 15, this ) );

  const QgsWkbTypes::Type geomTypes[] =
  {
    QgsWkbTypes::NoGeometry,
    QgsWkbTypes::Point,
    QgsWkbTypes::MultiPoint,
    QgsWkbTypes::LineString,
    QgsWkbTypes::Polygon,
  };

  for ( const auto type : geomTypes )
    mGeometryTypeBox->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), type );
  mGeometryTypeBox->setCurrentIndex( -1 );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  mFileFormatComboBox->addItem( tr( "ESRI Shapefile" ), "ESRI Shapefile" );
#if 0
  // Disabled until provider properly supports editing the created file formats
  // When enabling this, adapt the window-title of the dialog and the title of all actions showing this dialog.
  mFileFormatComboBox->addItem( tr( "Comma Separated Value" ), "Comma Separated Value" );
  mFileFormatComboBox->addItem( tr( "GML" ), "GML" );
  mFileFormatComboBox->addItem( tr( "Mapinfo File" ), "Mapinfo File" );
#endif
  if ( mFileFormatComboBox->count() == 1 )
  {
    mFileFormatComboBox->setVisible( false );
    mFileFormatLabel->setVisible( false );
  }

  mCrsSelector->setShowAccuracyWarnings( true );

  mFileFormatComboBox->setCurrentIndex( 0 );

  mFileEncoding->addItems( QgsVectorDataProvider::availableEncodings() );

  // Use default encoding if none supplied
  const QString enc = QgsSettings().value( QStringLiteral( "/UI/encoding" ), "System" ).toString();

  // The specified decoding is added if not existing already, and then set current.
  // This should select it.
  int encindex = mFileEncoding->findText( enc );
  if ( encindex < 0 )
  {
    mFileEncoding->insertItem( 0, enc );
    encindex = 0;
  }
  mFileEncoding->setCurrentIndex( encindex );

  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << QStringLiteral( "id" ) << QStringLiteral( "Integer" ) << QStringLiteral( "10" ) << QString() ) );
  connect( mNameEdit, &QLineEdit::textChanged, this, &QgsNewVectorLayerDialog::nameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewVectorLayerDialog::selectionChanged );
  connect( mGeometryTypeBox, static_cast<void( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ]( int )
  {
    updateExtension();
    checkOk();
  } );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

  mFileName->setStorageMode( QgsFileWidget::SaveFile );
  mFileName->setFilter( QgsVectorFileWriter::filterForDriver( mFileFormatComboBox->currentData( Qt::UserRole ).toString() ) );
  mFileName->setConfirmOverwrite( false );
  mFileName->setDialogTitle( tr( "Save Layer As" ) );
  const QgsSettings settings;
  mFileName->setDefaultRoot( settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString() );
  connect( mFileName, &QgsFileWidget::fileChanged, this, [ = ]
  {
    QgsSettings settings;
    const QFileInfo tmplFileInfo( mFileName->filePath() );
    settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), tmplFileInfo.absolutePath() );
    checkOk();
  } );
}

void QgsNewVectorLayerDialog::mFileFormatComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index )
  if ( mFileFormatComboBox->currentText() == tr( "ESRI Shapefile" ) )
    mNameEdit->setMaxLength( 10 );
  else
    mNameEdit->setMaxLength( 32767 );
}

void QgsNewVectorLayerDialog::mTypeBox_currentIndexChanged( int index )
{
  // FIXME: sync with providers/ogr/qgsogrprovider.cpp
  switch ( index )
  {
    case 0: // Text data
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 255 )
        mWidth->setText( QStringLiteral( "80" ) );
      mPrecision->setEnabled( false );
      mWidth->setValidator( new QIntValidator( 1, 255, this ) );
      break;

    case 1: // Whole number
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 10 )
        mWidth->setText( QStringLiteral( "10" ) );
      mPrecision->setEnabled( false );
      mWidth->setValidator( new QIntValidator( 1, 10, this ) );
      break;

    case 2: // Decimal number
      if ( mWidth->text().toInt() < 1 || mWidth->text().toInt() > 20 )
        mWidth->setText( QStringLiteral( "20" ) );
      if ( mPrecision->text().toInt() < 1 || mPrecision->text().toInt() > 15 )
        mPrecision->setText( QStringLiteral( "6" ) );

      mPrecision->setEnabled( true );
      mWidth->setValidator( new QIntValidator( 1, 20, this ) );
      break;

    default:
      QgsDebugMsg( QStringLiteral( "unexpected index" ) );
      break;
  }
}

QgsWkbTypes::Type QgsNewVectorLayerDialog::selectedType() const
{
  QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown;
  wkbType = static_cast<QgsWkbTypes::Type>
            ( mGeometryTypeBox->currentData( Qt::UserRole ).toInt() );

  if ( mGeometryWithZRadioButton->isChecked() )
    wkbType = QgsWkbTypes::addZ( wkbType );

  if ( mGeometryWithMRadioButton->isChecked() )
    wkbType = QgsWkbTypes::addM( wkbType );

  return wkbType;
}

QgsCoordinateReferenceSystem QgsNewVectorLayerDialog::crs() const
{
  return mCrsSelector->crs();
}

void QgsNewVectorLayerDialog::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  mCrsSelector->setCrs( crs );
}

void QgsNewVectorLayerDialog::mAddAttributeButton_clicked()
{
  const QString myName = mNameEdit->text();
  const QString myWidth = mWidth->text();
  const QString myPrecision = mPrecision->isEnabled() ? mPrecision->text() : QString();
  //use userrole to avoid translated type string
  const QString myType = mTypeBox->currentData( Qt::UserRole ).toString();
  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << myWidth << myPrecision ) );
  checkOk();
  mNameEdit->clear();
}

void QgsNewVectorLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();
  checkOk();
}

void QgsNewVectorLayerDialog::attributes( QList< QPair<QString, QString> > &at ) const
{
  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QTreeWidgetItem *item = *it;
    const QString type = QStringLiteral( "%1;%2;%3" ).arg( item->text( 1 ), item->text( 2 ), item->text( 3 ) );
    at.push_back( qMakePair( item->text( 0 ), type ) );
    QgsDebugMsg( QStringLiteral( "appending %1//%2" ).arg( item->text( 0 ), type ) );
    ++it;
  }
}

QString QgsNewVectorLayerDialog::selectedFileFormat() const
{
  //use userrole to avoid translated type string
  QString myType = mFileFormatComboBox->currentData( Qt::UserRole ).toString();
  return myType;
}

QString QgsNewVectorLayerDialog::selectedFileEncoding() const
{
  return mFileEncoding->currentText();
}

void QgsNewVectorLayerDialog::nameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || !mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewVectorLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

QString QgsNewVectorLayerDialog::filename() const
{
  return mFileName->filePath();
}

void QgsNewVectorLayerDialog::setFilename( const QString &filename )
{
  mFileName->setFilePath( filename );
}

void QgsNewVectorLayerDialog::checkOk()
{
  const bool ok = ( !mFileName->filePath().isEmpty() && mAttributeView->topLevelItemCount() > 0 && mGeometryTypeBox->currentIndex() != -1 );
  mOkButton->setEnabled( ok );
}

// this is static
QString QgsNewVectorLayerDialog::runAndCreateLayer( QWidget *parent, QString *pEnc, const QgsCoordinateReferenceSystem &crs, const QString &initialPath )
{
  QString error;
  QString res = execAndCreateLayer( error, parent, initialPath, pEnc, crs );
  if ( res.isEmpty() && error.isEmpty() )
    res = QString( "" ); // maintain gross earlier API compatibility
  return res;
}

void QgsNewVectorLayerDialog::updateExtension()
{
  QString fileName = filename();
  const QString fileformat = selectedFileFormat();
  const QgsWkbTypes::Type geometrytype = selectedType();
  if ( fileformat == QLatin1String( "ESRI Shapefile" ) )
  {
    if ( geometrytype != QgsWkbTypes::NoGeometry )
    {
      fileName = fileName.replace( fileName.lastIndexOf( QLatin1String( ".dbf" ), -1, Qt::CaseInsensitive ), 4, QLatin1String( ".shp" ) );
      fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { QStringLiteral( "shp" ) } );
    }
    else
    {
      fileName = fileName.replace( fileName.lastIndexOf( QLatin1String( ".shp" ), -1, Qt::CaseInsensitive ), 4, QLatin1String( ".dbf" ) );
      fileName = QgsFileUtils::ensureFileNameHasExtension( fileName, { QStringLiteral( "dbf" ) } );
    }
  }
  setFilename( fileName );

}

void QgsNewVectorLayerDialog::accept()
{
  updateExtension();

  if ( QFile::exists( filename() ) && QMessageBox::warning( this, tr( "New ShapeFile Layer" ), tr( "The layer already exists. Are you sure you want to overwrite the existing file?" ),
       QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel ) != QMessageBox::Yes )
    return;

  QDialog::accept();
}

QString QgsNewVectorLayerDialog::execAndCreateLayer( QString &errorMessage, QWidget *parent, const QString &initialPath, QString *encoding, const QgsCoordinateReferenceSystem &crs )
{
  errorMessage.clear();
  QgsNewVectorLayerDialog geomDialog( parent );
  geomDialog.setCrs( crs );
  if ( !initialPath.isEmpty() )
    geomDialog.setFilename( initialPath );
  if ( geomDialog.exec() == QDialog::Rejected )
  {
    return QString();
  }

  const QString fileformat = geomDialog.selectedFileFormat();
  const QgsWkbTypes::Type geometrytype = geomDialog.selectedType();
  QString fileName = geomDialog.filename();

  const QString enc = geomDialog.selectedFileEncoding();
  QgsDebugMsg( QStringLiteral( "New file format will be: %1" ).arg( fileformat ) );

  QList< QPair<QString, QString> > attributes;
  geomDialog.attributes( attributes );

  QgsSettings settings;
  settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), QFileInfo( fileName ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/encoding" ), enc );

  //try to create the new layer with OGRProvider instead of QgsVectorFileWriter
  if ( geometrytype != QgsWkbTypes::Unknown )
  {
    const QgsCoordinateReferenceSystem srs = geomDialog.crs();
    const bool success = QgsOgrProviderUtils::createEmptyDataSource( fileName, fileformat, enc, geometrytype, attributes, srs, errorMessage );
    if ( !success )
    {
      return QString();
    }
  }
  else
  {
    errorMessage = QObject::tr( "Geometry type not recognised" );
    QgsDebugMsg( errorMessage );
    return QString();
  }

  if ( encoding )
    *encoding = enc;

  return fileName;
}

void QgsNewVectorLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-shapefile-layer" ) );
}
