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
#include "qgis.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsproviderregistry.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorfilewriter.h"
#include "qgssettings.h"

#include <QPushButton>
#include <QComboBox>
#include <QLibrary>
#include <QFileDialog>


QgsNewVectorLayerDialog::QgsNewVectorLayerDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewVectorLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewVectorLayerDialog::mRemoveAttributeButton_clicked );
  connect( mFileFormatComboBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewVectorLayerDialog::mFileFormatComboBox_currentIndexChanged );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewVectorLayerDialog::mTypeBox_currentIndexChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewVectorLayerDialog::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/NewVectorLayer/geometry" ) ).toByteArray() );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mTypeBox->addItem( tr( "Text data" ), "String" );
  mTypeBox->addItem( tr( "Whole number" ), "Integer" );
  mTypeBox->addItem( tr( "Decimal number" ), "Real" );
  mTypeBox->addItem( tr( "Date" ), "Date" );

  mWidth->setValidator( new QIntValidator( 1, 255, this ) );
  mPrecision->setValidator( new QIntValidator( 0, 15, this ) );

  mPointRadioButton->setChecked( true );
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

  mFileFormatComboBox->setCurrentIndex( 0 );

  mFileEncoding->addItems( QgsVectorDataProvider::availableEncodings() );

  // Use default encoding if none supplied
  QString enc = QgsSettings().value( QStringLiteral( "/UI/encoding" ), "System" ).toString();

  // The specified decoding is added if not existing alread, and then set current.
  // This should select it.
  int encindex = mFileEncoding->findText( enc );
  if ( encindex < 0 )
  {
    mFileEncoding->insertItem( 0, enc );
    encindex = 0;
  }
  mFileEncoding->setCurrentIndex( encindex );

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );

  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << QStringLiteral( "id" ) << QStringLiteral( "Integer" ) << QStringLiteral( "10" ) << QLatin1String( "" ) ) );
  connect( mNameEdit, &QLineEdit::textChanged, this, &QgsNewVectorLayerDialog::nameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewVectorLayerDialog::selectionChanged );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );
}

QgsNewVectorLayerDialog::~QgsNewVectorLayerDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/NewVectorLayer/geometry" ), saveGeometry() );
}

void QgsNewVectorLayerDialog::mFileFormatComboBox_currentIndexChanged( int index )
{
  Q_UNUSED( index );
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
      mPrecision->setEnabled( true );
      mWidth->setValidator( new QIntValidator( 1, 20, this ) );
      break;

    default:
      QgsDebugMsg( "unexpected index" );
      break;
  }
}

QgsWkbTypes::Type QgsNewVectorLayerDialog::selectedType() const
{
  QgsWkbTypes::Type wkbType = QgsWkbTypes::Unknown;
  if ( mPointRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::Point;
  }
  else if ( mLineRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::LineString;
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    wkbType = QgsWkbTypes::Polygon;
  }

  if ( mGeometryWithZCheckBox->isChecked() && wkbType != QgsWkbTypes::Unknown )
    wkbType = QgsWkbTypes::to25D( wkbType );

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
  QString myName = mNameEdit->text();
  QString myWidth = mWidth->text();
  QString myPrecision = mPrecision->isEnabled() ? mPrecision->text() : QLatin1String( "" );
  //use userrole to avoid translated type string
  QString myType = mTypeBox->currentData( Qt::UserRole ).toString();
  mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType << myWidth << myPrecision ) );
  if ( mAttributeView->topLevelItemCount() > 0 )
  {
    mOkButton->setEnabled( true );
  }
  mNameEdit->clear();
}

void QgsNewVectorLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();
  if ( mAttributeView->topLevelItemCount() == 0 )
  {
    mOkButton->setEnabled( false );
  }
}

void QgsNewVectorLayerDialog::attributes( QList< QPair<QString, QString> > &at ) const
{
  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    QTreeWidgetItem *item = *it;
    QString type = QStringLiteral( "%1;%2;%3" ).arg( item->text( 1 ), item->text( 2 ), item->text( 3 ) );
    at.push_back( qMakePair( item->text( 0 ), type ) );
    QgsDebugMsg( QString( "appending %1//%2" ).arg( item->text( 0 ), type ) );
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


// this is static
QString QgsNewVectorLayerDialog::runAndCreateLayer( QWidget *parent, QString *pEnc, const QgsCoordinateReferenceSystem &crs )
{
  QgsNewVectorLayerDialog geomDialog( parent );
  geomDialog.setCrs( crs );
  if ( geomDialog.exec() == QDialog::Rejected )
  {
    return QLatin1String( "" );
  }

  QgsWkbTypes::Type geometrytype = geomDialog.selectedType();
  QString fileformat = geomDialog.selectedFileFormat();
  QString enc = geomDialog.selectedFileEncoding();
  QgsDebugMsg( QString( "New file format will be: %1" ).arg( fileformat ) );

  QList< QPair<QString, QString> > attributes;
  geomDialog.attributes( attributes );

  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastVectorFileFilterDir" ), QDir::homePath() ).toString();
  QString filterString = QgsVectorFileWriter::filterForDriver( fileformat );
  QString fileName = QFileDialog::getSaveFileName( nullptr, tr( "Save layer as..." ), lastUsedDir, filterString );
  if ( fileName.isNull() )
  {
    return QLatin1String( "" );
  }

  if ( fileformat == QLatin1String( "ESRI Shapefile" ) && !fileName.endsWith( QLatin1String( ".shp" ), Qt::CaseInsensitive ) )
    fileName += QLatin1String( ".shp" );

  settings.setValue( QStringLiteral( "UI/lastVectorFileFilterDir" ), QFileInfo( fileName ).absolutePath() );
  settings.setValue( QStringLiteral( "UI/encoding" ), enc );

  //try to create the new layer with OGRProvider instead of QgsVectorFileWriter
  QgsProviderRegistry *pReg = QgsProviderRegistry::instance();
  QString ogrlib = pReg->library( QStringLiteral( "ogr" ) );
  // load the data provider
  QLibrary *myLib = new QLibrary( ogrlib );
  bool loaded = myLib->load();
  if ( loaded )
  {
    QgsDebugMsg( "ogr provider loaded" );

    typedef bool ( *createEmptyDataSourceProc )( const QString &, const QString &, const QString &, QgsWkbTypes::Type,
        const QList< QPair<QString, QString> > &, const QgsCoordinateReferenceSystem & );
    createEmptyDataSourceProc createEmptyDataSource = ( createEmptyDataSourceProc ) cast_to_fptr( myLib->resolve( "createEmptyDataSource" ) );
    if ( createEmptyDataSource )
    {
      if ( geometrytype != QgsWkbTypes::Unknown )
      {
        QgsCoordinateReferenceSystem srs = geomDialog.crs();
        if ( !createEmptyDataSource( fileName, fileformat, enc, geometrytype, attributes, srs ) )
        {
          return QString();
        }
      }
      else
      {
        QgsDebugMsg( "geometry type not recognised" );
        return QString();
      }
    }
    else
    {
      QgsDebugMsg( "Resolving newEmptyDataSource(...) failed" );
      return QString();
    }
  }

  if ( pEnc )
    *pEnc = enc;

  return fileName;
}

void QgsNewVectorLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-shapefile-layer" ) );
}
