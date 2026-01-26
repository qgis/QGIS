/***************************************************************************
                          qgsrastercalcdialog.h  -  description
                          ---------------------
    begin                : September 28th, 2010
    copyright            : (C) 2010 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastercalcdialog.h"

#include <gdal.h>

#include "qgsgdalutils.h"
#include "qgsgui.h"
#include "qgshelp.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

#include <QFileDialog>
#include <QFontDatabase>

#include "moc_qgsrastercalcdialog.cpp"

QgsRasterCalcDialog::QgsRasterCalcDialog( QgsRasterLayer *rasterLayer, QgsMapCanvas *mapCanvas, QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mMapCanvas( mapCanvas )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mNColumnsSpinBox->setShowClearButton( false );
  mNRowsSpinBox->setShowClearButton( false );

  connect( mRasterBandsListWidget, &QListWidget::itemDoubleClicked, this, &QgsRasterCalcDialog::mRasterBandsListWidget_itemDoubleClicked );
  connect( mOutputFormatComboBox, &QComboBox::currentTextChanged, this, &QgsRasterCalcDialog::mOutputFormatComboBox_currentIndexChanged );
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentLayerChanged, this, &QgsRasterCalcDialog::extentLayerChanged );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRasterCalcDialog::mButtonBox_accepted );
  connect( mExpressionTextEdit, &QTextEdit::textChanged, this, &QgsRasterCalcDialog::mExpressionTextEdit_textChanged );
  connect( mPlusPushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mPlusPushButton_clicked );
  connect( mMinusPushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mMinusPushButton_clicked );
  connect( mMultiplyPushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mMultiplyPushButton_clicked );
  connect( mDividePushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mDividePushButton_clicked );
  connect( mSqrtButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mSqrtButton_clicked );
  connect( mCosButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mCosButton_clicked );
  connect( mSinButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mSinButton_clicked );
  connect( mASinButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mASinButton_clicked );
  connect( mExpButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mExpButton_clicked );
  connect( mLnButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mLnButton_clicked );
  connect( mLogButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mLogButton_clicked );
  connect( mNotEqualButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mNotEqualButton_clicked );
  connect( mTanButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mTanButton_clicked );
  connect( mACosButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mACosButton_clicked );
  connect( mATanButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mATanButton_clicked );
  connect( mOpenBracketPushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mOpenBracketPushButton_clicked );
  connect( mCloseBracketPushButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mCloseBracketPushButton_clicked );
  connect( mLessButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mLessButton_clicked );
  connect( mGreaterButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mGreaterButton_clicked );
  connect( mEqualButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mEqualButton_clicked );
  connect( mLesserEqualButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mLesserEqualButton_clicked );
  connect( mGreaterEqualButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mGreaterEqualButton_clicked );
  connect( mAndButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mAndButton_clicked );
  connect( mAbsButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mAbsButton_clicked );
  connect( mMinButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mMinButton_clicked );
  connect( mMaxButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mMaxButton_clicked );
  connect( mOrButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mOrButton_clicked );
  connect( mConditionalStatButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mConditionalStatButton_clicked );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterCalcDialog::showHelp );

  mExtentGroupBox->setCurrentExtent( mMapCanvas->extent(), mMapCanvas->mapSettings().destinationCrs() );

  if ( rasterLayer && rasterLayer->dataProvider() && rasterLayer->providerType() == "gdal"_L1 )
  {
    setExtentSize( rasterLayer );
    mCrsSelector->setCrs( rasterLayer->crs() );
    mExtentGroupBox->setOutputCrs( outputCrs() );
  }
  mCrsSelector->setShowAccuracyWarnings( true );

  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );

  //add supported output formats
  insertAvailableOutputFormats();
  insertAvailableRasterBands();

  mExpressionTextEdit->setCurrentFont( QFontDatabase::systemFont( QFontDatabase::FixedFont ) );

  QgsSettings settings;
  mOutputLayer->setStorageMode( QgsFileWidget::SaveFile );
  mOutputLayer->setDialogTitle( tr( "Enter Result File" ) );
  mOutputLayer->setDefaultRoot( settings.value( u"/RasterCalculator/lastOutputDir"_s, QDir::homePath() ).toString() );
  connect( mOutputLayer, &QgsFileWidget::fileChanged, this, [this]() { setAcceptButtonState(); } );

  connect( mUseVirtualProviderCheckBox, &QCheckBox::clicked, this, &QgsRasterCalcDialog::setOutputToVirtual );

  if ( !useVirtualProvider() )
  {
    setOutputToVirtual();
  }
}

QString QgsRasterCalcDialog::formulaString() const
{
  return mExpressionTextEdit->toPlainText();
}

QString QgsRasterCalcDialog::outputFile() const
{
  QString outputFileName = mOutputLayer->filePath();
  QFileInfo fileInfo( outputFileName );
  QString suffix = fileInfo.suffix();
  if ( !suffix.isEmpty() )
  {
    return outputFileName;
  }

  //add the file format extension if the user did not specify it
  int index = mOutputFormatComboBox->currentIndex();
  if ( index == -1 )
  {
    return outputFileName;
  }

  QString driverShortName = mOutputFormatComboBox->itemData( index ).toString();
  QMap<QString, QString>::const_iterator it = mDriverExtensionMap.find( driverShortName );
  if ( it == mDriverExtensionMap.constEnd() )
  {
    return outputFileName;
  }

  return outputFileName + '.' + it.value();
}

QString QgsRasterCalcDialog::outputFormat() const
{
  int index = mOutputFormatComboBox->currentIndex();
  if ( index == -1 )
  {
    return QString();
  }
  return mOutputFormatComboBox->itemData( index ).toString();
}

QgsCoordinateReferenceSystem QgsRasterCalcDialog::outputCrs() const
{
  return mCrsSelector->crs();
}

bool QgsRasterCalcDialog::addLayerToProject() const
{
  return mAddResultToProjectCheckBox->isChecked();
}

bool QgsRasterCalcDialog::useVirtualProvider() const
{
  return mUseVirtualProviderCheckBox->isChecked();
}

QString QgsRasterCalcDialog::virtualLayerName() const
{
  return mVirtualLayerName->text();
}

QVector<QgsRasterCalculatorEntry> QgsRasterCalcDialog::rasterEntries() const
{
  QVector<QgsRasterCalculatorEntry> entries;
  QString expressionString = mExpressionTextEdit->toPlainText();

  QList<QgsRasterCalculatorEntry>::const_iterator bandIt = mAvailableRasterBands.constBegin();
  for ( ; bandIt != mAvailableRasterBands.constEnd(); ++bandIt )
  {
    if ( expressionString.contains( bandIt->ref ) )
    {
      entries.push_back( *bandIt );
    }
  }

  return entries;
}

void QgsRasterCalcDialog::setExtentSize( QgsRasterLayer *layer )
{
  mNColumnsSpinBox->setValue( layer->width() );
  mNRowsSpinBox->setValue( layer->height() );
  mExtentGroupBox->setOutputExtentFromLayer( layer );
  mExtentSizeSet = true;
}

void QgsRasterCalcDialog::insertAvailableRasterBands()
{
  mAvailableRasterBands = QgsRasterCalculatorEntry::rasterEntries().toList();
  mRasterBandsListWidget->clear();
  for ( const auto &entry : std::as_const( mAvailableRasterBands ) )
  {
    QgsRasterLayer *rlayer = entry.raster;
    if ( !mExtentSizeSet ) //set bounding box / resolution of output to the values of the first possible input layer
    {
      setExtentSize( rlayer );
      mCrsSelector->setCrs( rlayer->crs() );
    }
    QListWidgetItem *item = new QListWidgetItem( entry.ref, mRasterBandsListWidget );
    item->setData( Qt::ToolTipRole, rlayer->publicSource() );
    mRasterBandsListWidget->addItem( item );
  }
}

void QgsRasterCalcDialog::insertAvailableOutputFormats()
{
  GDALAllRegister();

  int nDrivers = GDALGetDriverCount();
  for ( int i = 0; i < nDrivers; ++i )
  {
    GDALDriverH driver = GDALGetDriver( i );
    if ( driver )
    {
      if ( QgsGdalUtils::supportsRasterCreate( driver ) )
      {
        QString driverShortName = GDALGetDriverShortName( driver );
        QString driverLongName = GDALGetDriverLongName( driver );
        if ( driverShortName == "MEM"_L1 )
        {
          // in memory rasters are not (yet) supported because the GDAL dataset handle
          // would need to be passed directly to QgsRasterLayer (it is not possible to
          // close it in raster calculator and reopen the dataset again in raster layer)
          continue;
        }

        mOutputFormatComboBox->addItem( driverLongName, driverShortName );

        //store the driver shortnames and the corresponding extensions
        //(just in case the user does not give an extension for the output file name)
        QString driverExtension = GDALGetMetadataItem( driver, GDAL_DMD_EXTENSION, nullptr );
        mDriverExtensionMap.insert( driverShortName, driverExtension );
      }
    }
  }

  //and set last used driver in combo box
  QgsSettings s;
  QString lastUsedDriver = s.value( u"/RasterCalculator/lastOutputFormat"_s, "GeoTIFF" ).toString();
  int lastDriverIndex = mOutputFormatComboBox->findText( lastUsedDriver );
  if ( lastDriverIndex != -1 )
  {
    mOutputFormatComboBox->setCurrentIndex( lastDriverIndex );
  }
}

QgsRectangle QgsRasterCalcDialog::outputRectangle() const
{
  return mExtentGroupBox->outputExtent();
}

int QgsRasterCalcDialog::numberOfColumns() const
{
  return mNColumnsSpinBox->value();
}

int QgsRasterCalcDialog::numberOfRows() const
{
  return mNRowsSpinBox->value();
}

bool QgsRasterCalcDialog::outputLayerExists() const
{
  const QString layerName = QFileInfo( mOutputLayer->filePath() ).baseName();

  QString vectorUri;
  QString rasterUri;
  if ( outputFormat() == "GPKG"_L1 )
  {
    rasterUri = u"GPKG:%1:%2"_s.arg( outputFile(), layerName );
    vectorUri = u"%1|layername=%2"_s.arg( outputFile(), layerName );
  }
  else
  {
    rasterUri = outputFile();
  }

  QgsRasterLayer rasterLayer( rasterUri, QString(), u"gdal"_s );
  if ( !vectorUri.isEmpty() )
  {
    QgsVectorLayer vectorLayer( vectorUri, QString(), u"ogr"_s );
    return rasterLayer.isValid() || vectorLayer.isValid();
  }
  else
  {
    return rasterLayer.isValid();
  }
}

QStringList QgsRasterCalcDialog::creationOptions() const
{
  const QString layerName = QFileInfo( mOutputLayer->filePath() ).baseName();

  QStringList options = mCreationOptionsGroupBox->isChecked() ? mCreationOptionsWidget->options() : QStringList();
  if ( outputFormat() == "GPKG"_L1 )
  {
    // Overwrite the GPKG table options
    int indx = options.indexOf( QRegularExpression( "^RASTER_TABLE=.*", QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption ) );
    if ( indx > -1 )
    {
      options.replace( indx, u"RASTER_TABLE=%1"_s.arg( layerName ) );
    }
    else
    {
      options.append( u"RASTER_TABLE=%1"_s.arg( layerName ) );
    }

    // Only enable the append mode if the layer doesn't exist yet. For existing layers a 'confirm overwrite' dialog will be shown.
    if ( !outputLayerExists() )
    {
      indx = options.indexOf( QRegularExpression( "^APPEND_SUBDATASET=.*", QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption ) );
      if ( indx > -1 )
      {
        options.replace( indx, u"APPEND_SUBDATASET=YES"_s );
      }
      else
      {
        options.append( u"APPEND_SUBDATASET=YES"_s );
      }
    }
  }
  return options;
}

//slots

void QgsRasterCalcDialog::mButtonBox_accepted()
{
  //save last output format
  QgsSettings s;
  s.setValue( u"/RasterCalculator/lastOutputFormat"_s, QVariant( mOutputFormatComboBox->currentText() ) );
  s.setValue( u"/RasterCalculator/lastOutputDir"_s, QVariant( QFileInfo( mOutputLayer->filePath() ).absolutePath() ) );
}

void QgsRasterCalcDialog::showHelp()
{
  QgsHelp::openHelp( u"working_with_raster/raster_analysis.html#raster-calculator"_s );
}

void QgsRasterCalcDialog::extentLayerChanged( QgsMapLayer *layer )
{
  mCrsSelector->setCrs( layer->crs() );

  QgsRasterLayer *rasterLayer = qobject_cast<QgsRasterLayer *>( layer );
  if ( rasterLayer )
  {
    mNColumnsSpinBox->setValue( rasterLayer->width() );
    mNRowsSpinBox->setValue( rasterLayer->height() );
  }
}

void QgsRasterCalcDialog::mExpressionTextEdit_textChanged()
{
  if ( expressionValid() )
  {
    mExpressionValidLabel->setText( tr( "Expression valid" ) );
    if ( filePathValid() || useVirtualProvider() )
    {
      mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
      return;
    }
  }
  else
  {
    mExpressionValidLabel->setText( tr( "Expression invalid" ) );
  }
  mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
}

void QgsRasterCalcDialog::mOutputFormatComboBox_currentIndexChanged( const QString & )
{
  mCreationOptionsWidget->setFormat( outputFormat() );
  mCreationOptionsWidget->update();
}

void QgsRasterCalcDialog::setAcceptButtonState()
{
  if ( ( expressionValid() && filePathValid() ) || ( expressionValid() && useVirtualProvider() ) )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
  else
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  }
}

void QgsRasterCalcDialog::setOutputToVirtual()
{
  if ( useVirtualProvider() )
  {
    mOutputFormatComboBox->hide();
    mOutputLayer->hide();
    mOutputLayerLabel->hide();
    mOutputFormatLabel->hide();
    mAddResultToProjectCheckBox->isChecked();
    mAddResultToProjectCheckBox->setEnabled( false );
    mVirtualLayerLabel->show();
    mVirtualLayerName->show();
    setAcceptButtonState();
  }
  else
  {
    mOutputFormatComboBox->show();
    mOutputLayer->show();
    mOutputLayerLabel->show();
    mOutputFormatLabel->show();
    mAddResultToProjectCheckBox->setEnabled( true );
    mVirtualLayerLabel->hide();
    mVirtualLayerName->hide();
    setAcceptButtonState();
  }
}

bool QgsRasterCalcDialog::expressionValid() const
{
  QString errorString;
  QgsRasterCalcNode *testNode = QgsRasterCalcNode::parseRasterCalcString( mExpressionTextEdit->toPlainText(), errorString );
  if ( testNode )
  {
    delete testNode;
    return true;
  }
  return false;
}

bool QgsRasterCalcDialog::filePathValid() const
{
  QString outputPath = mOutputLayer->filePath();
  if ( outputPath.isEmpty() )
    return false;

  outputPath = QFileInfo( outputPath ).absolutePath();
  return QFileInfo( outputPath ).isWritable();
}

void QgsRasterCalcDialog::mRasterBandsListWidget_itemDoubleClicked( QListWidgetItem *item )
{
  mExpressionTextEdit->insertPlainText( quoteBandEntry( item->text() ) );
  //to enable the "ok" button if someone checks the virtual provider checkbox before adding a valid expression,
  if ( expressionValid() && useVirtualProvider() )
    setAcceptButtonState();
}

void QgsRasterCalcDialog::mPlusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" + "_s );
}

void QgsRasterCalcDialog::mMinusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" - "_s );
}

void QgsRasterCalcDialog::mMultiplyPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" * "_s );
}

void QgsRasterCalcDialog::mDividePushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" / "_s );
}

void QgsRasterCalcDialog::mSqrtButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" sqrt ( "_s );
}

void QgsRasterCalcDialog::mCosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" cos ( "_s );
}

void QgsRasterCalcDialog::mSinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" sin ( "_s );
}

void QgsRasterCalcDialog::mASinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" asin ( "_s );
}

void QgsRasterCalcDialog::mExpButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ^ "_s );
}

void QgsRasterCalcDialog::mTanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" tan ( "_s );
}

void QgsRasterCalcDialog::mACosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" acos ( "_s );
}

void QgsRasterCalcDialog::mATanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" atan ( "_s );
}

void QgsRasterCalcDialog::mLnButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ln ( "_s );
}

void QgsRasterCalcDialog::mLogButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" log10 ( "_s );
}

void QgsRasterCalcDialog::mNotEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" != "_s );
}

void QgsRasterCalcDialog::mOpenBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ( "_s );
}

void QgsRasterCalcDialog::mCloseBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ) "_s );
}

void QgsRasterCalcDialog::mLessButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" < "_s );
}

void QgsRasterCalcDialog::mGreaterButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" > "_s );
}

void QgsRasterCalcDialog::mEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" = "_s );
}

void QgsRasterCalcDialog::mLesserEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" <= "_s );
}

void QgsRasterCalcDialog::mGreaterEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" >= "_s );
}

void QgsRasterCalcDialog::mAndButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" AND "_s );
}

void QgsRasterCalcDialog::mOrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" OR "_s );
}

void QgsRasterCalcDialog::mAbsButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" ABS ( "_s );
}

void QgsRasterCalcDialog::mMinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" MIN ( "_s );
}

void QgsRasterCalcDialog::mMaxButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" MAX ( "_s );
}

void QgsRasterCalcDialog::mConditionalStatButton_clicked()
{
  mExpressionTextEdit->insertPlainText( u" if ( "_s );
}

QString QgsRasterCalcDialog::quoteBandEntry( const QString &layerName )
{
  // '"' -> '\\"'
  QString quotedName = layerName;
  quotedName.replace( '\"', "\\\""_L1 );
  quotedName.append( '\"' );
  quotedName.prepend( '\"' );
  return quotedName;
}
