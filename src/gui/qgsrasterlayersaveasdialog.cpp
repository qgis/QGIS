/***************************************************************************
    qgsrasterlayersaveasdialog.cpp
    ---------------------
    begin                : May 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco dot hugentobler at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsapplication.h"
#include "qgsgdalutils.h"
#include "qgslogger.h"
#include "qgscoordinatetransform.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayersaveasdialog.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterformatsaveoptionswidget.h"
#include "qgsrasterrenderer.h"
#include "qgsrastertransparency.h"
#include "qgsprojectionselectiondialog.h"
#include "qgssettings.h"
#include "qgsrasterfilewriter.h"
#include "qgsvectorlayer.h"
#include "cpl_string.h"
#include "qgsproject.h"
#include <gdal.h>
#include "qgsmessagelog.h"
#include "qgsgui.h"
#include "qgsdoublevalidator.h"
#include "qgsdatums.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

QgsRasterLayerSaveAsDialog::QgsRasterLayerSaveAsDialog( QgsRasterLayer *rasterLayer,
    QgsRasterDataProvider *sourceProvider, const QgsRectangle &currentExtent,
    const QgsCoordinateReferenceSystem &layerCrs, const QgsCoordinateReferenceSystem &currentCrs,
    QWidget *parent, Qt::WindowFlags f )
  : QDialog( parent, f )
  , mRasterLayer( rasterLayer )
  , mDataProvider( sourceProvider )
  , mCurrentExtent( currentExtent )
  , mLayerCrs( layerCrs )
  , mCurrentCrs( currentCrs )
  , mResolutionState( OriginalResolution )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
  connect( mRawModeRadioButton, &QRadioButton::toggled, this, &QgsRasterLayerSaveAsDialog::mRawModeRadioButton_toggled );
  connect( mFormatComboBox, &QComboBox::currentTextChanged, this, &QgsRasterLayerSaveAsDialog::mFormatComboBox_currentIndexChanged );
  connect( mResolutionRadioButton, &QRadioButton::toggled, this, &QgsRasterLayerSaveAsDialog::mResolutionRadioButton_toggled );
  connect( mOriginalResolutionPushButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mOriginalResolutionPushButton_clicked );
  connect( mXResolutionLineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerSaveAsDialog::mXResolutionLineEdit_textEdited );
  connect( mYResolutionLineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerSaveAsDialog::mYResolutionLineEdit_textEdited );
  connect( mOriginalSizePushButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mOriginalSizePushButton_clicked );
  connect( mColumnsLineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerSaveAsDialog::mColumnsLineEdit_textEdited );
  connect( mRowsLineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerSaveAsDialog::mRowsLineEdit_textEdited );
  connect( mAddNoDataManuallyToolButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mAddNoDataManuallyToolButton_clicked );
  connect( mLoadTransparentNoDataToolButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mLoadTransparentNoDataToolButton_clicked );
  connect( mRemoveSelectedNoDataToolButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mRemoveSelectedNoDataToolButton_clicked );
  connect( mRemoveAllNoDataToolButton, &QPushButton::clicked, this, &QgsRasterLayerSaveAsDialog::mRemoveAllNoDataToolButton_clicked );
  connect( mTileModeCheckBox, &QCheckBox::toggled, this, &QgsRasterLayerSaveAsDialog::mTileModeCheckBox_toggled );
  connect( mPyramidsGroupBox, &QgsCollapsibleGroupBox::toggled, this, &QgsRasterLayerSaveAsDialog::mPyramidsGroupBox_toggled );
  mAddNoDataManuallyToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyAdd.svg" ) ) );
  mLoadTransparentNoDataToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  mRemoveSelectedNoDataToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/symbologyRemove.svg" ) ) );
  mRemoveAllNoDataToolButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionRemove.svg" ) ) );

  mNoDataTableWidget->setColumnCount( 2 );
  mNoDataTableWidget->setHorizontalHeaderItem( 0, new QTableWidgetItem( tr( "From" ) ) );
  mNoDataTableWidget->setHorizontalHeaderItem( 1, new QTableWidgetItem( tr( "To" ) ) );

  mRawModeRadioButton_toggled( true );

  setValidators();

  toggleResolutionSize();

  insertAvailableOutputFormats();

  //fill reasonable default values depending on the provider
  if ( mDataProvider )
  {
    if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size )
    {
      setOriginalResolution();
      int xSize = mDataProvider->xSize();
      int ySize = mDataProvider->ySize();
      mMaximumSizeXLineEdit->setText( QString::number( xSize ) );
      mMaximumSizeYLineEdit->setText( QString::number( ySize ) );
    }
    else //wms, sometimes wcs
    {
      mTileModeCheckBox->setChecked( true );
      mMaximumSizeXLineEdit->setText( QString::number( 2000 ) );
      mMaximumSizeYLineEdit->setText( QString::number( 2000 ) );
    }

    // setup creation option widget
    mCreateOptionsWidget->setProvider( mDataProvider->name() );
    if ( mDataProvider->name() == QLatin1String( "gdal" ) )
    {
      mCreateOptionsWidget->setFormat( mFormatComboBox->currentData().toString() );
    }
    mCreateOptionsWidget->setRasterLayer( mRasterLayer );
    mCreateOptionsWidget->update();
  }

  // Only do pyramids if dealing directly with GDAL.
  if ( mDataProvider && mDataProvider->capabilities() & QgsRasterDataProvider::BuildPyramids )
  {
    // setup pyramids option widget
    // mPyramidsOptionsWidget->createOptionsWidget()->setType( QgsRasterFormatSaveOptionsWidget::ProfileLineEdit );
    mPyramidsOptionsWidget->createOptionsWidget()->setRasterLayer( mRasterLayer );

    // TODO enable "use existing", has no effect for now, because using Create() in gdal provider
    // if ( ! mDataProvider->hasPyramids() )
    //   mPyramidsButtonGroup->button( QgsRaster::PyramidsCopyExisting )->setEnabled( false );
    mPyramidsUseExistingCheckBox->setEnabled( false );
    mPyramidsUseExistingCheckBox->setVisible( false );

    populatePyramidsLevels();
    connect( mPyramidsOptionsWidget, &QgsRasterPyramidsOptionsWidget::overviewListChanged,
             this, &QgsRasterLayerSaveAsDialog::populatePyramidsLevels );
  }
  else
  {
    mPyramidsGroupBox->setEnabled( false );
  }

  // restore checked state for most groupboxes (default is to restore collapsed state)
  // create options and pyramids will be preset, if user has selected defaults in the gdal options dlg
  mCreateOptionsGroupBox->setSaveCheckedState( true );
  //mTilesGroupBox->setSaveCheckedState( true );
  // don't restore nodata, it needs user input
  // pyramids are not necessarily built every time

  try
  {
    const QgsDatumEnsemble ensemble = mLayerCrs.datumEnsemble();
    if ( ensemble.isValid() )
    {
      mCrsSelector->setSourceEnsemble( ensemble.name() );
    }
  }
  catch ( QgsNotSupportedException & )
  {
  }
  mCrsSelector->setShowAccuracyWarnings( true );

  mCrsSelector->setLayerCrs( mLayerCrs );
  //default to layer CRS - see https://github.com/qgis/QGIS/issues/22211 for discussion
  mCrsSelector->setCrs( mLayerCrs );

  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged,
           this, &QgsRasterLayerSaveAsDialog::crsChanged );

  QPushButton *okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( false );
  }

#ifdef Q_OS_WIN
  mHelpButtonBox->setVisible( false );
  mButtonBox->addButton( QDialogButtonBox::Help );
  connect( mButtonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterLayerSaveAsDialog::showHelp );
#else
  connect( mHelpButtonBox, &QDialogButtonBox::helpRequested, this, &QgsRasterLayerSaveAsDialog::showHelp );
#endif
  connect( mButtonBox, &QDialogButtonBox::accepted, this, &QgsRasterLayerSaveAsDialog::accept );
  connect( mButtonBox, &QDialogButtonBox::rejected, this, &QgsRasterLayerSaveAsDialog::reject );

  mExtentGroupBox->setOutputCrs( outputCrs() );
  mExtentGroupBox->setOriginalExtent( mDataProvider->extent(), mLayerCrs );
  mExtentGroupBox->setCurrentExtent( mCurrentExtent, mCurrentCrs );
  mExtentGroupBox->setOutputExtentFromOriginal();
  connect( mExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsRasterLayerSaveAsDialog::extentChanged );

  recalcResolutionSize();

  QgsSettings settings;

  if ( mTileModeCheckBox->isChecked() )
  {
    mTilesGroupBox->show();
    mFilename->setStorageMode( QgsFileWidget::GetDirectory );
    mFilename->setDialogTitle( tr( "Select Output Directory" ) );
  }
  else
  {
    mTilesGroupBox->hide();
    mFilename->setStorageMode( QgsFileWidget::SaveFile );
    mFilename->setDialogTitle( tr( "Save Layer As" ) );
  }

  mFilename->setDefaultRoot( settings.value( QStringLiteral( "UI/lastRasterFileDir" ), QDir::homePath() ).toString() );
  connect( mFilename, &QgsFileWidget::fileChanged, this, [ = ]( const QString & filePath )
  {
    QgsSettings settings;
    QFileInfo tmplFileInfo( filePath );
    settings.setValue( QStringLiteral( "UI/lastRasterFileDir" ), tmplFileInfo.absolutePath() );

    if ( !filePath.isEmpty() && mLayerName->isEnabled() )
    {
      QFileInfo fileInfo( filePath );
      mLayerName->setText( fileInfo.baseName() );
    }

    if ( mTileModeCheckBox->isChecked() )
    {
      QString fileName = filePath;
      Q_FOREVER
      {
        // TODO: would not it be better to select .vrt file instead of directory?
        //fileName = QFileDialog::getSaveFileName( this, tr( "Select output file" ), QString(), tr( "VRT" ) + " (*.vrt *.VRT)" );
        if ( fileName.isEmpty() )
          break; // canceled

        // Check if directory is empty
        QDir dir( fileName );
        QString baseName = QFileInfo( fileName ).baseName();
        QStringList filters;
        filters << QStringLiteral( "%1.*" ).arg( baseName );
        QStringList files = dir.entryList( filters );
        if ( files.isEmpty() )
          break;

        if ( QMessageBox::warning( this, tr( "Save Raster Layer" ),
                                   tr( "The directory %1 contains files which will be overwritten: %2" ).arg( dir.absolutePath(), files.join( QLatin1String( ", " ) ) ),
                                   QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Ok )
          break;

        fileName = QFileDialog::getExistingDirectory( this, tr( "Select output directory" ), tmplFileInfo.absolutePath() );
      }
    }

    QPushButton *okButton = mButtonBox->button( QDialogButtonBox::Ok );
    if ( !okButton )
    {
      return;
    }
    okButton->setEnabled( tmplFileInfo.absoluteDir().exists() );
  } );
}

void QgsRasterLayerSaveAsDialog::insertAvailableOutputFormats()
{
  GDALAllRegister();

  int nDrivers = GDALGetDriverCount();
  QMap< int, QPair< QString, QString > > topPriorityDrivers;
  QMap< QString, QString > lowPriorityDrivers;

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
        else if ( driverShortName == QLatin1String( "VRT" ) )
        {
          // skip GDAL vrt driver, since we handle that format manually
          continue;
        }
        else if ( driverShortName == QLatin1String( "GTiff" ) )
        {
          // always list geotiff first
          topPriorityDrivers.insert( 1, qMakePair( driverLongName, driverShortName ) );
        }
        else if ( driverShortName == QLatin1String( "GPKG" ) )
        {
          // and gpkg second
          topPriorityDrivers.insert( 2, qMakePair( driverLongName, driverShortName ) );
        }
        else
        {
          lowPriorityDrivers.insert( driverLongName, driverShortName );
        }
      }
    }
  }

  // will be sorted by priority, so that geotiff and geopackage are listed first
  for ( auto priorityDriversIt = topPriorityDrivers.constBegin(); priorityDriversIt != topPriorityDrivers.constEnd(); ++priorityDriversIt )
  {
    mFormatComboBox->addItem( priorityDriversIt.value().first, priorityDriversIt.value().second );
  }
  // will be sorted by driver name
  for ( auto lowPriorityDriversIt = lowPriorityDrivers.constBegin(); lowPriorityDriversIt != lowPriorityDrivers.constEnd(); ++lowPriorityDriversIt )
  {
    mFormatComboBox->addItem( lowPriorityDriversIt.key(), lowPriorityDriversIt.value() );
  }

}

void QgsRasterLayerSaveAsDialog::setValidators()
{
  mXResolutionLineEdit->setValidator( new QgsDoubleValidator( this ) );
  mYResolutionLineEdit->setValidator( new QgsDoubleValidator( this ) );
  mColumnsLineEdit->setValidator( new QIntValidator( this ) );
  mRowsLineEdit->setValidator( new QIntValidator( this ) );
  mMaximumSizeXLineEdit->setValidator( new QIntValidator( this ) );
  mMaximumSizeYLineEdit->setValidator( new QIntValidator( this ) );
}

void QgsRasterLayerSaveAsDialog::mFormatComboBox_currentIndexChanged( const QString & )
{
  //gdal-specific
  if ( mDataProvider && mDataProvider->name() == QLatin1String( "gdal" ) )
  {
    mCreateOptionsWidget->setFormat( outputFormat() );
    mCreateOptionsWidget->update();
  }

  QStringList extensions = QgsRasterFileWriter::extensionsForFormat( outputFormat() );
  QString filter;
  if ( extensions.empty() )
    filter = tr( "All files (*.*)" );
  else
  {
    filter = QStringLiteral( "%1 (*.%2);;%3" ).arg( mFormatComboBox->currentText(),
             extensions.join( QLatin1String( " *." ) ),
             tr( "All files (*.*)" ) );
  }
  mFilename->setFilter( filter );

  // Disable mTileModeCheckBox for GeoPackages
  mTileModeCheckBox->setEnabled( outputFormat() != QLatin1String( "GPKG" ) );
  mFilename->setConfirmOverwrite( outputFormat() != QLatin1String( "GPKG" ) );
  mLayerName->setEnabled( outputFormat() == QLatin1String( "GPKG" ) );
  if ( mLayerName->isEnabled() )
  {
    QString layerName = QFileInfo( mFilename->filePath() ).baseName();
    mLayerName->setText( layerName );
    mTileModeCheckBox->setChecked( false );
  }
  else
  {
    mLayerName->setText( QString() );
  }
}

int QgsRasterLayerSaveAsDialog::nColumns() const
{
  return mColumnsLineEdit->text().toInt();
}

int QgsRasterLayerSaveAsDialog::nRows() const
{
  return mRowsLineEdit->text().toInt();
}

double QgsRasterLayerSaveAsDialog::xResolution() const
{
  return QgsDoubleValidator::toDouble( mXResolutionLineEdit->text() );
}

double QgsRasterLayerSaveAsDialog::yResolution() const
{
  return QgsDoubleValidator::toDouble( mYResolutionLineEdit->text() );
}

int QgsRasterLayerSaveAsDialog::maximumTileSizeX() const
{
  return mMaximumSizeXLineEdit->text().toInt();
}

int QgsRasterLayerSaveAsDialog::maximumTileSizeY() const
{
  return mMaximumSizeYLineEdit->text().toInt();
}

bool QgsRasterLayerSaveAsDialog::tileMode() const
{
  return mTileModeCheckBox->isChecked();
}

bool QgsRasterLayerSaveAsDialog::addToCanvas() const
{
  return mAddToCanvas->isChecked();
}

void QgsRasterLayerSaveAsDialog::setAddToCanvas( bool checked )
{
  mAddToCanvas->setChecked( checked );
}

QString QgsRasterLayerSaveAsDialog::outputFileName() const
{
  QString fileName = mFilename->filePath();

  if ( mFilename->storageMode() != QgsFileWidget::GetDirectory )
  {
    QStringList extensions = QgsRasterFileWriter::extensionsForFormat( outputFormat() );
    QString defaultExt;
    if ( !extensions.empty() )
    {
      defaultExt = extensions.at( 0 );
    }

    // ensure the user never omits the extension from the file name
    QFileInfo fi( fileName );
    if ( !fileName.isEmpty() && fi.suffix().isEmpty() && !defaultExt.isEmpty() )
    {
      fileName += '.' + defaultExt;
    }
  }

  return fileName;
}

QString QgsRasterLayerSaveAsDialog::outputLayerName() const
{
  if ( mLayerName->text().isEmpty() && outputFormat() == QLatin1String( "GPKG" ) && !mTileModeCheckBox->isChecked() )
  {
    // Always return layer name for GeoPackages
    return QFileInfo( mFilename->filePath() ).baseName();
  }
  else
  {
    return mLayerName->text();
  }
}

QString QgsRasterLayerSaveAsDialog::outputFormat() const
{
  return mFormatComboBox->currentData().toString();
}

QStringList QgsRasterLayerSaveAsDialog::createOptions() const
{
  QStringList options = mCreateOptionsGroupBox->isChecked() ? mCreateOptionsWidget->options() : QStringList();
  if ( outputFormat() == QLatin1String( "GPKG" ) )
  {
    // Overwrite the GPKG table options
    int indx = options.indexOf( QRegularExpression( "^RASTER_TABLE=.*", QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption ) );
    if ( indx > -1 )
    {
      options.replace( indx, QStringLiteral( "RASTER_TABLE=%1" ).arg( outputLayerName() ) );
    }
    else
    {
      options.append( QStringLiteral( "RASTER_TABLE=%1" ).arg( outputLayerName() ) );
    }

    // Only enable the append mode if the layer doesn't exist yet. For existing layers a 'confirm overwrite' dialog will be shown.
    if ( !outputLayerExists() )
    {
      indx = options.indexOf( QRegularExpression( "^APPEND_SUBDATASET=.*", QRegularExpression::CaseInsensitiveOption | QRegularExpression::MultilineOption ) );
      if ( indx > -1 )
      {
        options.replace( indx, QStringLiteral( "APPEND_SUBDATASET=YES" ) );
      }
      else
      {
        options.append( QStringLiteral( "APPEND_SUBDATASET=YES" ) );
      }
    }
  }
  return options;
}

QgsRectangle QgsRasterLayerSaveAsDialog::outputRectangle() const
{
  return mExtentGroupBox->outputExtent();
}

void QgsRasterLayerSaveAsDialog::hideFormat()
{
  mFormatLabel->hide();
  mFormatComboBox->hide();
}

void QgsRasterLayerSaveAsDialog::hideOutput()
{
  mSaveAsLabel->hide();
  mFilename->hide();
  QPushButton *okButton = mButtonBox->button( QDialogButtonBox::Ok );
  if ( okButton )
  {
    okButton->setEnabled( true );
  }
}

void QgsRasterLayerSaveAsDialog::toggleResolutionSize()
{
  bool hasResolution = mDataProvider && mDataProvider->capabilities() & QgsRasterDataProvider::Size;

  bool on = mResolutionRadioButton->isChecked();
  mXResolutionLineEdit->setEnabled( on );
  mYResolutionLineEdit->setEnabled( on );
  mOriginalResolutionPushButton->setEnabled( on && hasResolution );
  mColumnsLineEdit->setEnabled( !on );
  mRowsLineEdit->setEnabled( !on );
  mOriginalSizePushButton->setEnabled( !on && hasResolution );
}

void QgsRasterLayerSaveAsDialog::setOriginalResolution()
{
  double xRes, yRes;

  if ( mDataProvider->capabilities() & QgsRasterDataProvider::Size )
  {
    xRes = mDataProvider->extent().width() / mDataProvider->xSize();
    yRes = mDataProvider->extent().height() / mDataProvider->ySize();
  }
  else
  {
    // Init to something if no original resolution is available
    xRes = yRes = mDataProvider->extent().width() / 100;
  }
  setResolution( xRes, yRes, mLayerCrs );
  mResolutionState = OriginalResolution;
  recalcSize();
}

void QgsRasterLayerSaveAsDialog::setResolution( double xRes, double yRes, const QgsCoordinateReferenceSystem &srcCrs )
{
  if ( srcCrs != outputCrs() )
  {
    // We reproject pixel rectangle from center of selected extent, of course, it gives
    // bigger xRes,yRes than reprojected edges (envelope), it may also be that
    // close to margins are higher resolutions (even very, too high)
    // TODO: consider more precise resolution calculation

    QgsPointXY center = outputRectangle().center();
    QgsCoordinateTransform ct( srcCrs, outputCrs(), QgsProject::instance() );
    QgsPointXY srsCenter = ct.transform( center, Qgis::TransformDirection::Reverse );

    QgsRectangle srcExtent( srsCenter.x() - xRes / 2, srsCenter.y() - yRes / 2, srsCenter.x() + xRes / 2, srsCenter.y() + yRes / 2 );

    QgsRectangle extent = ct.transform( srcExtent );
    xRes = extent.width();
    yRes = extent.height();
  }
  mXResolutionLineEdit->setText( QLocale().toString( xRes ) );
  mYResolutionLineEdit->setText( QLocale().toString( yRes ) );
}

void QgsRasterLayerSaveAsDialog::recalcSize()
{
  QgsRectangle extent = outputRectangle();
  int xSize = xResolution() != 0 ? static_cast<int>( std::round( extent.width() / xResolution() ) ) : 0;
  int ySize = yResolution() != 0 ? static_cast<int>( std::round( extent.height() / yResolution() ) ) : 0;
  mColumnsLineEdit->setText( QString::number( xSize ) );
  mRowsLineEdit->setText( QString::number( ySize ) );
  updateResolutionStateMsg();
}

void QgsRasterLayerSaveAsDialog::setOriginalSize()
{
  mColumnsLineEdit->setText( QString::number( mDataProvider->xSize() ) );
  mRowsLineEdit->setText( QString::number( mDataProvider->ySize() ) );
  recalcResolution();
}

void QgsRasterLayerSaveAsDialog::recalcResolution()
{
  QgsRectangle extent = outputRectangle();
  double xRes = nColumns() != 0 ? extent.width() / nColumns() : 0;
  double yRes = nRows() != 0 ? extent.height() / nRows() : 0;
  mXResolutionLineEdit->setText( QLocale().toString( xRes ) );
  mYResolutionLineEdit->setText( QLocale().toString( yRes ) );
  updateResolutionStateMsg();
}

void QgsRasterLayerSaveAsDialog::recalcResolutionSize()
{
  if ( mResolutionRadioButton->isChecked() )
  {
    recalcSize();
  }
  else
  {
    mResolutionState = UserResolution;
    recalcResolution();
  }
}

void QgsRasterLayerSaveAsDialog::updateResolutionStateMsg()
{
  QString msg;
  switch ( mResolutionState )
  {
    case OriginalResolution:
      msg = tr( "layer" );
      break;
    case UserResolution:
      msg = tr( "user defined" );
      break;
    default:
      break;
  }
  msg = tr( "Resolution (current: %1)" ).arg( msg );
  mResolutionGroupBox->setTitle( msg );
}

void QgsRasterLayerSaveAsDialog::extentChanged()
{
  // Whenever extent changes with fixed size, original resolution is lost
  if ( mSizeRadioButton->isChecked() )
  {
    mResolutionState = UserResolution;
  }
  recalcResolutionSize();
}

void QgsRasterLayerSaveAsDialog::crsChanged()
{
  if ( outputCrs() != mPreviousCrs )
  {
    mExtentGroupBox->setOutputCrs( outputCrs() );

    // Reset resolution
    if ( mResolutionRadioButton->isChecked() )
    {
      if ( mResolutionState == OriginalResolution )
      {
        setOriginalResolution();
      }
      else
      {
        // reset from present resolution and present crs
        setResolution( xResolution(), yResolution(), mPreviousCrs );
      }
    }
    else
    {
      // Size does not change, we just recalc resolution from new extent
      recalcResolution();
    }
  }
  mPreviousCrs = outputCrs();
}

QgsCoordinateReferenceSystem QgsRasterLayerSaveAsDialog::outputCrs()
{
  return mCrsSelector->crs();
}

QgsRasterLayerSaveAsDialog::Mode QgsRasterLayerSaveAsDialog::mode() const
{
  if ( mRenderedModeRadioButton->isChecked() ) return RenderedImageMode;
  return RawDataMode;
}

void QgsRasterLayerSaveAsDialog::mRawModeRadioButton_toggled( bool checked )
{
  mNoDataGroupBox->setEnabled( checked && mDataProvider->bandCount() == 1 );
}

void QgsRasterLayerSaveAsDialog::mAddNoDataManuallyToolButton_clicked()
{
  addNoDataRow( std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN() );
}

void QgsRasterLayerSaveAsDialog::mLoadTransparentNoDataToolButton_clicked()
{
  if ( !mRasterLayer->renderer() ) return;
  const QgsRasterTransparency *rasterTransparency = mRasterLayer->renderer()->rasterTransparency();
  if ( !rasterTransparency ) return;

  const auto constTransparentSingleValuePixelList = rasterTransparency->transparentSingleValuePixelList();
  for ( const QgsRasterTransparency::TransparentSingleValuePixel &transparencyPixel : constTransparentSingleValuePixelList )
  {
    if ( transparencyPixel.percentTransparent == 100 )
    {
      addNoDataRow( transparencyPixel.min, transparencyPixel.max );
      if ( transparencyPixel.min != transparencyPixel.max )
      {
        setNoDataToEdited( mNoDataTableWidget->rowCount() - 1 );
      }
    }
  }
}

void QgsRasterLayerSaveAsDialog::mRemoveSelectedNoDataToolButton_clicked()
{
  mNoDataTableWidget->removeRow( mNoDataTableWidget->currentRow() );
}

void QgsRasterLayerSaveAsDialog::mRemoveAllNoDataToolButton_clicked()
{
  while ( mNoDataTableWidget->rowCount() > 0 )
  {
    mNoDataTableWidget->removeRow( 0 );
  }
}

void QgsRasterLayerSaveAsDialog::addNoDataRow( double min, double max )
{
  mNoDataTableWidget->insertRow( mNoDataTableWidget->rowCount() );
  for ( int i = 0; i < 2; i++ )
  {
    double value = i == 0 ? min : max;
    QLineEdit *lineEdit = new QLineEdit();
    lineEdit->setFrame( false );
    lineEdit->setContentsMargins( 1, 1, 1, 1 );
    QString valueString;
    switch ( mRasterLayer->dataProvider()->sourceDataType( 1 ) )
    {
      case Qgis::DataType::Float32:
      case Qgis::DataType::Float64:
        lineEdit->setValidator( new QgsDoubleValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          valueString = QgsRasterBlock::printValue( value );
        }
        break;
      default:
        lineEdit->setValidator( new QIntValidator( nullptr ) );
        if ( !std::isnan( value ) )
        {
          valueString = QLocale().toString( static_cast<int>( value ) );
        }
        break;
    }
    lineEdit->setText( valueString );
    mNoDataTableWidget->setCellWidget( mNoDataTableWidget->rowCount() - 1, i, lineEdit );

    adjustNoDataCellWidth( mNoDataTableWidget->rowCount() - 1, i );

    connect( lineEdit, &QLineEdit::textEdited, this, &QgsRasterLayerSaveAsDialog::noDataCellTextEdited );
  }
  mNoDataTableWidget->resizeColumnsToContents();
  mNoDataTableWidget->resizeRowsToContents();
}

void QgsRasterLayerSaveAsDialog::noDataCellTextEdited( const QString &text )
{
  Q_UNUSED( text )

  QLineEdit *lineEdit = qobject_cast<QLineEdit *>( sender() );
  if ( !lineEdit ) return;
  int row = -1;
  int column = -1;
  for ( int r = 0; r < mNoDataTableWidget->rowCount(); r++ )
  {
    for ( int c = 0; c < mNoDataTableWidget->columnCount(); c++ )
    {
      if ( mNoDataTableWidget->cellWidget( r, c ) == sender() )
      {
        row = r;
        column = c;
        break;
      }
    }
    if ( row != -1 ) break;
  }
  QgsDebugMsg( QStringLiteral( "row = %1 column =%2" ).arg( row ).arg( column ) );

  if ( column == 0 )
  {
    QLineEdit *toLineEdit = dynamic_cast<QLineEdit *>( mNoDataTableWidget->cellWidget( row, 1 ) );
    if ( !toLineEdit ) return;
    bool toChanged = mNoDataToEdited.value( row );
    QgsDebugMsg( QStringLiteral( "toChanged = %1" ).arg( toChanged ) );
    if ( !toChanged )
    {
      toLineEdit->setText( lineEdit->text() );
    }
  }
  else if ( column == 1 )
  {
    setNoDataToEdited( row );
  }
}

void QgsRasterLayerSaveAsDialog::mTileModeCheckBox_toggled( bool toggled )
{
  if ( toggled )
  {
    // enable pyramids

    // Disabled (Radim), auto enabling of pyramids was making impression that
    // we (programmers) know better what you (user) want to do,
    // certainly auto expanding was a bad experience

    //if ( ! mPyramidsGroupBox->isChecked() )
    //  mPyramidsGroupBox->setChecked( true );

    // Auto expanding mPyramidsGroupBox is bad - it auto scrolls content of dialog
    //if ( mPyramidsGroupBox->isCollapsed() )
    //  mPyramidsGroupBox->setCollapsed( false );
    //mPyramidsOptionsWidget->checkAllLevels( true );

    // Show / hide tile options
    mTilesGroupBox->show();
    mFilename->setStorageMode( QgsFileWidget::GetDirectory );
    mFilename->setDialogTitle( tr( "Select Output Directory" ) );
  }
  else
  {
    mTilesGroupBox->hide();
    mFilename->setStorageMode( QgsFileWidget::SaveFile );
    mFilename->setDialogTitle( tr( "Save Layer As" ) );
  }
}

void QgsRasterLayerSaveAsDialog::mPyramidsGroupBox_toggled( bool toggled )
{
  Q_UNUSED( toggled )
  populatePyramidsLevels();
}

void QgsRasterLayerSaveAsDialog::populatePyramidsLevels()
{
  QString text;

  if ( mPyramidsGroupBox->isChecked() )
  {
    QList<QgsRasterPyramid> myPyramidList;
    // if use existing, get pyramids from actual layer
    // but that's not available yet
    if ( mPyramidsUseExistingCheckBox->isChecked() )
    {
      myPyramidList = mDataProvider->buildPyramidList();
    }
    else
    {
      if ( ! mPyramidsOptionsWidget->overviewList().isEmpty() )
        myPyramidList = mDataProvider->buildPyramidList( mPyramidsOptionsWidget->overviewList() );
    }
    for ( const QgsRasterPyramid &pyramid : std::as_const( myPyramidList ) )
    {
      if ( ! mPyramidsUseExistingCheckBox->isChecked() || pyramid.getExists() )
      {
        text += QString::number( pyramid.getXDim() ) + QStringLiteral( "x" ) +
                QString::number( pyramid.getYDim() ) + ' ';
      }
    }
  }

  mPyramidResolutionsLineEdit->setText( text.trimmed() );
}

void QgsRasterLayerSaveAsDialog::setNoDataToEdited( int row )
{
  if ( row >= mNoDataToEdited.size() )
  {
    mNoDataToEdited.resize( row + 1 );
  }
  mNoDataToEdited[row] = true;
}

double QgsRasterLayerSaveAsDialog::noDataCellValue( int row, int column ) const
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( mNoDataTableWidget->cellWidget( row, column ) );
  if ( !lineEdit || lineEdit->text().isEmpty() )
  {
    return std::numeric_limits<double>::quiet_NaN();
  }
  return QgsDoubleValidator::toDouble( lineEdit->text() );
}

void QgsRasterLayerSaveAsDialog::adjustNoDataCellWidth( int row, int column )
{
  QLineEdit *lineEdit = dynamic_cast<QLineEdit *>( mNoDataTableWidget->cellWidget( row, column ) );
  if ( !lineEdit ) return;

  int width = std::max( lineEdit->fontMetrics().boundingRect( lineEdit->text() ).width() + 10, 100 );
  width = std::max( width, mNoDataTableWidget->columnWidth( column ) );

  lineEdit->setFixedWidth( width );
}

QgsRasterRangeList QgsRasterLayerSaveAsDialog::noData() const
{
  QgsRasterRangeList noDataList;
  if ( ! mNoDataGroupBox->isChecked() )
    return noDataList;

  int rows = mNoDataTableWidget->rowCount();
  noDataList.reserve( rows );
  for ( int r = 0; r < rows; r++ )
  {
    QgsRasterRange noData( noDataCellValue( r, 0 ), noDataCellValue( r, 1 ) );
    noDataList.append( noData );

  }
  return noDataList;
}

QList<int> QgsRasterLayerSaveAsDialog::pyramidsList() const
{
  return mPyramidsGroupBox->isChecked() ? mPyramidsOptionsWidget->overviewList() : QList<int>();
}

QgsRaster::RasterBuildPyramids QgsRasterLayerSaveAsDialog::buildPyramidsFlag() const
{
  if ( ! mPyramidsGroupBox->isChecked() )
    return QgsRaster::PyramidsFlagNo;
  else if ( mPyramidsUseExistingCheckBox->isChecked() )
    return QgsRaster::PyramidsCopyExisting;
  else
    return QgsRaster::PyramidsFlagYes;
}

bool QgsRasterLayerSaveAsDialog::validate() const
{
  if ( mCreateOptionsGroupBox->isChecked() )
  {
    QString message = mCreateOptionsWidget->validateOptions( true, false );
    if ( !message.isNull() )
      return false;
  }
  if ( mPyramidsGroupBox->isChecked() )
  {
    QString message = mPyramidsOptionsWidget->createOptionsWidget()->validateOptions( true, false );
    if ( !message.isNull() )
      return false;
  }
  return true;
}

bool QgsRasterLayerSaveAsDialog::outputLayerExists() const
{
  QString vectorUri;
  QString rasterUri;
  if ( outputFormat() == QLatin1String( "GPKG" ) )
  {
    rasterUri = QStringLiteral( "GPKG:%1:%2" ).arg( outputFileName(), outputLayerName() );
    vectorUri = QStringLiteral( "%1|layername=%2" ).arg( outputFileName(), outputLayerName() );
  }
  else
  {
    rasterUri = outputFileName();
  }

  QgsRasterLayer rasterLayer( rasterUri, QString( ), QStringLiteral( "gdal" ) );
  if ( !vectorUri.isEmpty() )
  {
    QgsVectorLayer vectorLayer( vectorUri, QString( ), QStringLiteral( "ogr" ) );
    return rasterLayer.isValid() || vectorLayer.isValid();
  }
  else
  {
    return rasterLayer.isValid();
  }
}

void QgsRasterLayerSaveAsDialog::accept()
{
  if ( !validate() )
  {
    return;
  }

  if ( outputFormat() == QLatin1String( "GPKG" ) && outputLayerExists() &&
       QMessageBox::warning( this, tr( "Save Raster Layer" ),
                             tr( "The layer %1 already exists in the target file, and overwriting layers in GeoPackage is not supported. "
                                 "Do you want to overwrite the whole file?" ).arg( outputLayerName() ),
                             QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
  {
    return;
  }

  QDialog::accept();
}

void QgsRasterLayerSaveAsDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-new-layers-from-an-existing-layer" ) );
}
