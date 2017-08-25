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
#include "qgsproject.h"
#include "qgsrastercalcnode.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"
#include "qgssettings.h"

#include "cpl_string.h"
#include "gdal.h"

#include <QFileDialog>

QgsRasterCalcDialog::QgsRasterCalcDialog( QWidget *parent, Qt::WindowFlags f ): QDialog( parent, f )
{
  setupUi( this );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterCalcDialog::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/RasterCalc/geometry" ) ).toByteArray() );

  //add supported output formats
  insertAvailableOutputFormats();
  insertAvailableRasterBands();

  if ( !mAvailableRasterBands.isEmpty() )
  {
    //grab default crs from first raster
    mCrsSelector->setCrs( mAvailableRasterBands.at( 0 ).raster->crs() );
  }
}

QgsRasterCalcDialog::~QgsRasterCalcDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/RasterCalc/geometry" ), saveGeometry() );
}

QString QgsRasterCalcDialog::formulaString() const
{
  return mExpressionTextEdit->toPlainText();
}

QString QgsRasterCalcDialog::outputFile() const
{
  QString outputFileName = mOutputLayerLineEdit->text();
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
    return QLatin1String( "" );
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

void QgsRasterCalcDialog::insertAvailableRasterBands()
{
  const QMap<QString, QgsMapLayer *> &layers = QgsProject::instance()->mapLayers();
  QMap<QString, QgsMapLayer *>::const_iterator layerIt = layers.constBegin();

  bool firstLayer = true;
  for ( ; layerIt != layers.constEnd(); ++layerIt )
  {
    QgsRasterLayer *rlayer = dynamic_cast<QgsRasterLayer *>( layerIt.value() );
    if ( rlayer && rlayer->dataProvider() && rlayer->dataProvider()->name() == QLatin1String( "gdal" ) )
    {
      if ( firstLayer ) //set bounding box / resolution of output to the values of the first possible input layer
      {
        mNColumnsSpinBox->setValue( rlayer->width() );
        mNRowsSpinBox->setValue( rlayer->height() );
        QgsRectangle bbox = rlayer->extent();
        mXMinSpinBox->setValue( bbox.xMinimum() );
        mXMaxSpinBox->setValue( bbox.xMaximum() );
        mYMinSpinBox->setValue( bbox.yMinimum() );
        mYMaxSpinBox->setValue( bbox.yMaximum() );
        firstLayer = false;
      }
      //get number of bands
      for ( int i = 0; i < rlayer->bandCount(); ++i )
      {
        QgsRasterCalculatorEntry entry;
        entry.raster = rlayer;
        entry.bandNumber = i + 1;
        entry.ref = rlayer->name() + '@' + QString::number( i + 1 );
        mAvailableRasterBands.push_back( entry );
        mRasterBandsListWidget->addItem( entry.ref );
      }
    }
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
      char **driverMetadata = GDALGetMetadata( driver, nullptr );
      if ( CSLFetchBoolean( driverMetadata, GDAL_DCAP_CREATE, false ) )
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

void QgsRasterCalcDialog::on_mButtonBox_accepted()
{
  //save last output format
  QgsSettings s;
  s.setValue( QStringLiteral( "/RasterCalculator/lastOutputFormat" ), QVariant( mOutputFormatComboBox->currentText() ) );
  s.setValue( QStringLiteral( "/RasterCalculator/lastOutputDir" ), QVariant( QFileInfo( mOutputLayerLineEdit->text() ).absolutePath() ) );
}

void QgsRasterCalcDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/raster_analysis.html#raster-calculator" ) );
}

void QgsRasterCalcDialog::on_mOutputLayerPushButton_clicked()
{
  QgsSettings s;
  QString saveFileName = QFileDialog::getSaveFileName( nullptr, tr( "Enter result file" ), s.value( QStringLiteral( "/RasterCalculator/lastOutputDir" ), QDir::homePath() ).toString() );
  if ( !saveFileName.isNull() )
  {
    mOutputLayerLineEdit->setText( saveFileName );
  }
}

void QgsRasterCalcDialog::on_mCurrentLayerExtentButton_clicked()
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

void QgsRasterCalcDialog::on_mExpressionTextEdit_textChanged()
{
  if ( expressionValid() )
  {
    mExpressionValidLabel->setText( tr( "Expression valid" ) );
    if ( filePathValid() )
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

void QgsRasterCalcDialog::on_mOutputLayerLineEdit_textChanged( const QString &text )
{
  Q_UNUSED( text );
  setAcceptButtonState();
}

void QgsRasterCalcDialog::setAcceptButtonState()
{
  if ( expressionValid() && filePathValid() )
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( true );
  }
  else
  {
    mButtonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
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
  QString outputPath = mOutputLayerLineEdit->text();
  if ( outputPath.isEmpty() )
    return false;

  outputPath = QFileInfo( outputPath ).absolutePath();
  return QFileInfo( outputPath ).isWritable();
}

void QgsRasterCalcDialog::on_mRasterBandsListWidget_itemDoubleClicked( QListWidgetItem *item )
{
  mExpressionTextEdit->insertPlainText( quoteBandEntry( item->text() ) );
}

void QgsRasterCalcDialog::on_mPlusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " + " ) );
}

void QgsRasterCalcDialog::on_mMinusPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " - " ) );
}

void QgsRasterCalcDialog::on_mMultiplyPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " * " ) );
}

void QgsRasterCalcDialog::on_mDividePushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " / " ) );
}

void QgsRasterCalcDialog::on_mSqrtButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " std::sqrt ( " ) );
}

void QgsRasterCalcDialog::on_mCosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " std::cos ( " ) );
}

void QgsRasterCalcDialog::on_mSinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " sin ( " ) );
}

void QgsRasterCalcDialog::on_mASinButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " asin ( " ) );
}

void QgsRasterCalcDialog::on_mExpButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ^ " ) );
}

void QgsRasterCalcDialog::on_mTanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " tan ( " ) );
}

void QgsRasterCalcDialog::on_mACosButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " acos ( " ) );
}

void QgsRasterCalcDialog::on_mATanButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " atan ( " ) );
}

void QgsRasterCalcDialog::on_mLnButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ln ( " ) );
}

void QgsRasterCalcDialog::on_mLogButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " log10 ( " ) );
}

void QgsRasterCalcDialog::on_mNotEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " != " ) );
}

void QgsRasterCalcDialog::on_mOpenBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ( " ) );
}

void QgsRasterCalcDialog::on_mCloseBracketPushButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " ) " ) );
}

void QgsRasterCalcDialog::on_mLessButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " < " ) );
}

void QgsRasterCalcDialog::on_mGreaterButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " > " ) );
}

void QgsRasterCalcDialog::on_mEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " = " ) );
}

void QgsRasterCalcDialog::on_mLesserEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " <= " ) );
}

void QgsRasterCalcDialog::on_mGreaterEqualButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " >= " ) );
}

void QgsRasterCalcDialog::on_mAndButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " AND " ) );
}

void QgsRasterCalcDialog::on_mOrButton_clicked()
{
  mExpressionTextEdit->insertPlainText( QStringLiteral( " OR " ) );
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
