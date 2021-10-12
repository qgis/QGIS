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

#include "qgsgdalutils.h"
#include "qgsrastercalcdialog.h"
#include "qgsproject.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"
#include "qgsgui.h"

#include "cpl_string.h"
#include "gdal.h"

#include <QFileDialog>
#include <QFontDatabase>

QgsRasterCalcDialog::QgsRasterCalcDialog( QgsRasterLayer *rasterLayer, QWidget *parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  mXMaxSpinBox->setShowClearButton( false );
  mXMinSpinBox->setShowClearButton( false );
  mYMaxSpinBox->setShowClearButton( false );
  mYMinSpinBox->setShowClearButton( false );
  mNColumnsSpinBox->setShowClearButton( false );
  mNRowsSpinBox->setShowClearButton( false );

  connect( mRasterBandsListWidget, &QListWidget::itemDoubleClicked, this, &QgsRasterCalcDialog::mRasterBandsListWidget_itemDoubleClicked );
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRasterCalcDialog::mButtonBox_accepted );
  connect( mCurrentLayerExtentButton, &QPushButton::clicked, this, &QgsRasterCalcDialog::mCurrentLayerExtentButton_clicked );
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

  if ( rasterLayer && rasterLayer->dataProvider() && rasterLayer->providerType() == QLatin1String( "gdal" ) )
  {
    setExtentSize( rasterLayer->width(), rasterLayer->height(), rasterLayer->extent() );
    mCrsSelector->setCrs( rasterLayer->crs() );
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
  mOutputLayer->setDefaultRoot( settings.value( QStringLiteral( "/RasterCalculator/lastOutputDir" ), QDir::homePath() ).toString() );
  connect( mOutputLayer, &QgsFileWidget::fileChanged, this, [ = ]() { setAcceptButtonState(); } );

  connect( mUseVirtualProviderCheckBox, &QCheckBox::clicked, this, &QgsRasterCalcDialog::setOutputToVirtual );

  if ( ! useVirtualProvider() )
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


void QgsRasterCalcDialog::setExtentSize( int width, int height, QgsRectangle bbox )
{
  mNColumnsSpinBox->setValue( width );
  mNRowsSpinBox->setValue( height );
  mXMinSpinBox->setValue( bbox.xMinimum() );
  mXMaxSpinBox->setValue( bbox.xMaximum() );
  mYMinSpinBox->setValue( bbox.yMinimum() );
  mYMaxSpinBox->setValue( bbox.yMaximum() );
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
      setExtentSize( rlayer->width(), rlayer->height(), rlayer->extent() );
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
        if ( driverShortName == QLatin1String( "MEM" ) )
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
  QString lastUsedDriver = s.value( QStringLiteral( "/RasterCalculator/lastOutputFormat" ), "GeoTIFF" ).toString();
  int lastDriverIndex = mOutputFormatComboBox->findText( lastUsedDriver );
  if ( lastDriverIndex != -1 )
  {
    mOutputFormatComboBox->setCurrentIndex( lastDriverIndex );
  }
}

QgsRectangle QgsRasterCalcDialog::outputRectangle() const
{
  return QgsRectangle( mXMinSpinBox->value(), mYMinSpinBox->value(), mXMaxSpinBox->value(), mYMaxSpinBox->value() );
}

int QgsRasterCalcDialog::numberOfColumns() const
{
  return mNColumnsSpinBox->value();
}

int QgsRasterCalcDialog::numberOfRows() const
{
  return mNRowsSpinBox->value();
}

//slots

void QgsRasterCalcDialog::mButtonBox_accepted()
{
  //save last output format
  QgsSettings s;
  s.setValue( QStringLiteral( "/RasterCalculator/lastOutputFormat" ), QVariant( mOutputFormatComboBox->currentText() ) );
  s.setValue( QStringLiteral( "/RasterCalculator/lastOutputDir" ), QVariant( QFileInfo( mOutputLayer->filePath() ).absolutePath() ) );
}

void QgsRasterCalcDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/raster_analysis.html#raster-calculator" ) );
}

void QgsRasterCalcDialog::mCurrentLayerExtentButton_clicked()
{
  QListWidgetItem *currentLayerItem = mRasterBandsListWidget->currentItem();
  if ( currentLayerItem )
  {
    QgsRasterLayer *rlayer = nullptr;
    QList<QgsRasterCalculatorEntry>::const_iterator rasterIt = mAvailableRasterBands.constBegin();
    for ( ; rasterIt != mAvailableRasterBands.constEnd(); ++rasterIt )
    {
      if ( rasterIt->ref == currentLayerItem->text() )
      {
        rlayer = rasterIt->raster;
      }
    }

    if ( !rlayer )
    {
      return;
    }

    QgsRectangle layerExtent = rlayer->extent();
    mXMinSpinBox->setValue( layerExtent.xMinimum() );
    mXMaxSpinBox->setValue( layerExtent.xMaximum() );
    mYMinSpinBox->setValue( layerExtent.yMinimum() );
    mYMaxSpinBox->setValue( layerExtent.yMaximum() );
    mNColumnsSpinBox->setValue( rlayer->width() );
    mNRowsSpinBox->setValue( rlayer->height() );
    mCrsSelector->setCrs( rlayer->crs() );
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
  if ( expressionValid() && useVirtualProvider() ) setAcceptButtonState();
}

void QgsRasterCalcDialog::mPlusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " + " ) );
}

void QgsRasterCalcDialog::mMinusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " - " ) );
}

void QgsRasterCalcDialog::mMultiplyPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " * " ) );
}

void QgsRasterCalcDialog::mDividePushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " / " ) );
}

void QgsRasterCalcDialog::mSqrtButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " sqrt ( " ) );
}

void QgsRasterCalcDialog::mCosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " cos ( " ) );
}

void QgsRasterCalcDialog::mSinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " sin ( " ) );
}

void QgsRasterCalcDialog::mASinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " asin ( " ) );
}

void QgsRasterCalcDialog::mExpButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ^ " ) );
}

void QgsRasterCalcDialog::mTanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " tan ( " ) );
}

void QgsRasterCalcDialog::mACosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " acos ( " ) );
}

void QgsRasterCalcDialog::mATanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " atan ( " ) );
}

void QgsRasterCalcDialog::mLnButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ln ( " ) );
}

void QgsRasterCalcDialog::mLogButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " log10 ( " ) );
}

void QgsRasterCalcDialog::mNotEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " != " ) );
}

void QgsRasterCalcDialog::mOpenBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ( " ) );
}

void QgsRasterCalcDialog::mCloseBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ) " ) );
}

void QgsRasterCalcDialog::mLessButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " < " ) );
}

void QgsRasterCalcDialog::mGreaterButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " > " ) );
}

void QgsRasterCalcDialog::mEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " = " ) );
}

void QgsRasterCalcDialog::mLesserEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " <= " ) );
}

void QgsRasterCalcDialog::mGreaterEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " >= " ) );
}

void QgsRasterCalcDialog::mAndButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " AND " ) );
}

void QgsRasterCalcDialog::mOrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " OR " ) );
}

void QgsRasterCalcDialog::mAbsButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ABS ( " ) );
}

void QgsRasterCalcDialog::mMinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " MIN ( " ) );
}

void QgsRasterCalcDialog::mMaxButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " MAX ( " ) );
}

void QgsRasterCalcDialog::mConditionalStatButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " if ( " ) );
}

QString QgsRasterCalcDialog::quoteBandEntry( const QString &layerName )
{
  // '"' -> '\\"'
  QString quotedName = layerName;
  quotedName.replace( '\"', QLatin1String( "\\\"" ) );
  quotedName.append( '\"' );
  quotedName.prepend( '\"' );
  return quotedName;
}
