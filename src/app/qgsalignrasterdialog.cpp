/***************************************************************************
    qgsalignrasterdialog.cpp
    ---------------------
    begin                : June 2015
    copyright            : (C) 2015 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsalignrasterdialog.h"

#include "qgisapp.h"
#include "qgsapplication.h"
#include "qgsalignraster.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayercombobox.h"
#include "qgsproject.h"
#include "qgsrasterlayer.h"
#include "qgsiconutils.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>


static QgsMapLayer *_rasterLayer( const QString &filename )
{
  const QMap<QString, QgsMapLayer *> layers = QgsProject::instance()->mapLayers();
  const auto constLayers = layers;
  for ( QgsMapLayer *layer : constLayers )
  {
    if ( layer->type() == QgsMapLayerType::RasterLayer && layer->source() == filename )
      return layer;
  }
  return nullptr;
}

static QString _rasterLayerName( const QString &filename )
{
  if ( QgsMapLayer *layer = _rasterLayer( filename ) )
    return layer->name();

  const QFileInfo fi( filename );
  return fi.baseName();
}



//! Helper class to report progress
struct QgsAlignRasterDialogProgress : public QgsAlignRaster::ProgressHandler
{
    explicit QgsAlignRasterDialogProgress( QProgressBar *pb ) : mPb( pb ) {}
    bool progress( double complete ) override
    {
      mPb->setValue( ( int ) std::round( complete * 100 ) );
      qApp->processEvents(); // to actually show the progress in GUI
      return true;
    }

  protected:
    QProgressBar *mPb = nullptr;
};


QgsAlignRasterDialog::QgsAlignRasterDialog( QWidget *parent )
  : QDialog( parent )
{
  setupUi( this );

  mBtnAdd->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mBtnEdit->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.svg" ) ) );
  mBtnRemove->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  mAlign = new QgsAlignRaster;
  mAlign->setProgressHandler( new QgsAlignRasterDialogProgress( mProgress ) );

  mCrsSelector->setShowAccuracyWarnings( true );

  connect( mBtnAdd, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::addLayer );
  connect( mBtnRemove, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::removeLayer );
  connect( mBtnEdit, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::editLayer );

  connect( mCboReferenceLayer, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, [ = ] { referenceLayerChanged(); } );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsAlignRasterDialog::destinationCrsChanged );
  connect( mSpinCellSizeX, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsAlignRasterDialog::updateParametersFromReferenceLayer );
  connect( mSpinCellSizeY, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsAlignRasterDialog::updateParametersFromReferenceLayer );
  connect( mSpinGridOffsetX, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsAlignRasterDialog::updateParametersFromReferenceLayer );
  connect( mSpinGridOffsetY, static_cast < void ( QDoubleSpinBox::* )( double ) > ( &QDoubleSpinBox::valueChanged ), this, &QgsAlignRasterDialog::updateParametersFromReferenceLayer );

  connect( mChkCustomCRS, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::updateCustomCrs );
  connect( mChkCustomCellSize, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::updateCustomCellSize );
  connect( mChkCustomGridOffset, &QAbstractButton::clicked, this, &QgsAlignRasterDialog::updateCustomGridOffset );

  mClipExtentGroupBox->setChecked( false );
  mClipExtentGroupBox->setCollapsed( true );
  mClipExtentGroupBox->setTitleBase( tr( "Clip to Extent" ) );
  QgsMapCanvas *mc = QgisApp::instance()->mapCanvas();
  mClipExtentGroupBox->setCurrentExtent( mc->extent(), mc->mapSettings().destinationCrs() );
  connect( mClipExtentGroupBox, &QgsExtentGroupBox::extentChanged, this, &QgsAlignRasterDialog::clipExtentChanged );

  // TODO: auto-detect reference layer

  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsAlignRasterDialog::runAlign );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsAlignRasterDialog::showHelp );

  populateLayersView();

  updateCustomCrs();
  updateCustomCellSize();
  updateCustomGridOffset();
}

QgsAlignRasterDialog::~QgsAlignRasterDialog()
{
  delete mAlign;
}


void QgsAlignRasterDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_raster/raster_analysis.html#raster-alignment" ) );
}


void QgsAlignRasterDialog::populateLayersView()
{
  mCboReferenceLayer->clear();

  const int refLayerIndex = mAlign->suggestedReferenceLayer();

  QStandardItemModel *model = new QStandardItemModel();
  int i = 0;
  const auto constRasters = mAlign->rasters();
  for ( const QgsAlignRaster::Item &item : constRasters )
  {
    QString layerName = _rasterLayerName( item.inputFilename );

    QStandardItem *si = new QStandardItem( QgsIconUtils::iconRaster(), layerName );
    model->appendRow( si );

    if ( i == refLayerIndex )
      layerName += tr( " [best reference]" );

    mCboReferenceLayer->addItem( layerName );
    ++i;
  }

  mViewLayers->setModel( model );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( model->rowCount() > 0 );

  if ( refLayerIndex >= 0 )
    mCboReferenceLayer->setCurrentIndex( refLayerIndex );

  updateAlignedRasterInfo();
}


void QgsAlignRasterDialog::updateAlignedRasterInfo()
{
  if ( !mAlign->checkInputParameters() )
  {
    mEditOutputSize->setText( mAlign->errorMessage() );
    return;
  }

  const QSize size = mAlign->alignedRasterSize();
  const QString msg = QStringLiteral( "%1 x %2" ).arg( size.width() ).arg( size.height() );
  mEditOutputSize->setText( msg );
}

void QgsAlignRasterDialog::updateParametersFromReferenceLayer()
{
  QString customCRSWkt;
  QSizeF customCellSize;
  QPointF customGridOffset( -1, -1 );

  const int index = mCboReferenceLayer->currentIndex();
  if ( index < 0 )
    return;

  const QgsAlignRaster::RasterInfo refInfo( mAlign->rasters().at( index ).inputFilename );
  if ( !refInfo.isValid() )
    return;

  // get custom values from the GUI (if any)
  if ( mChkCustomCRS->isChecked() )
  {
    const QgsCoordinateReferenceSystem refCRS( refInfo.crs() );
    if ( refCRS != mCrsSelector->crs() )
      customCRSWkt = mCrsSelector->crs( ).toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL );
  }

  if ( mChkCustomCellSize->isChecked() )
  {
    customCellSize = QSizeF( mSpinCellSizeX->value(), mSpinCellSizeY->value() );
  }

  if ( mChkCustomGridOffset->isChecked() )
  {
    customGridOffset = QPointF( mSpinGridOffsetX->value(), mSpinGridOffsetY->value() );
  }

  // calculate the parameters which are not customized already
  const bool res = mAlign->setParametersFromRaster( refInfo, customCRSWkt, customCellSize, customGridOffset );

  // refresh values that may have changed
  if ( res )
  {
    const QgsCoordinateReferenceSystem destCRS( mAlign->destinationCrs() );
    mClipExtentGroupBox->setOutputCrs( destCRS );
    if ( !mChkCustomCRS->isChecked() )
    {
      mCrsSelector->setCrs( destCRS );
    }
  }
  if ( !mChkCustomCellSize->isChecked() )
  {
    const QSizeF cellSize = mAlign->cellSize();
    mSpinCellSizeX->setValue( cellSize.width() );
    mSpinCellSizeY->setValue( cellSize.height() );
  }
  if ( !mChkCustomGridOffset->isChecked() )
  {
    const QPointF gridOffset = mAlign->gridOffset();
    mSpinGridOffsetX->setValue( gridOffset.x() );
    mSpinGridOffsetY->setValue( gridOffset.y() );
  }

  updateAlignedRasterInfo();
}


void QgsAlignRasterDialog::addLayer()
{
  QgsAlignRasterLayerConfigDialog d;
  if ( !d.exec() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();

  QgsAlignRaster::Item item( d.inputFilename(), d.outputFilename() );
  item.resampleMethod = d.resampleMethod();
  item.rescaleValues = d.rescaleValues();
  list.append( item );

  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::removeLayer()
{
  const QModelIndex current = mViewLayers->currentIndex();
  if ( !current.isValid() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();
  list.removeAt( current.row() );
  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::editLayer()
{
  const QModelIndex current = mViewLayers->currentIndex();
  if ( !current.isValid() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();
  const QgsAlignRaster::Item item = list.at( current.row() );

  QgsAlignRasterLayerConfigDialog d;
  d.setItem( item.inputFilename, item.outputFilename, item.resampleMethod, item.rescaleValues );
  if ( !d.exec() )
    return;

  QgsAlignRaster::Item itemNew( d.inputFilename(), d.outputFilename() );
  itemNew.resampleMethod = d.resampleMethod();
  itemNew.rescaleValues = d.rescaleValues();
  list[current.row()] = itemNew;
  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::referenceLayerChanged()
{
  const int index = mCboReferenceLayer->currentIndex();
  if ( index < 0 )
    return;

  const QgsAlignRaster::RasterInfo refInfo( mAlign->rasters().at( index ).inputFilename );
  if ( !refInfo.isValid() )
    return;

  const QgsCoordinateReferenceSystem layerCRS( refInfo.crs() );
  mCrsSelector->setLayerCrs( layerCRS );
  mClipExtentGroupBox->setOriginalExtent( refInfo.extent(), layerCRS );

  updateParametersFromReferenceLayer();
}


void QgsAlignRasterDialog::destinationCrsChanged()
{
  if ( mCrsSelector->crs().toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED_GDAL ) == mAlign->destinationCrs() )
    return;

  const int index = mCboReferenceLayer->currentIndex();
  if ( index < 0 )
    return;

  const QgsAlignRaster::RasterInfo refInfo( mAlign->rasters().at( index ).inputFilename );
  if ( !refInfo.isValid() )
    return;

  updateParametersFromReferenceLayer();
}

void QgsAlignRasterDialog::clipExtentChanged()
{
  mAlign->setClipExtent( mClipExtentGroupBox->outputExtent() );

  updateAlignedRasterInfo();
}

void QgsAlignRasterDialog::updateCustomCrs()
{
  mCrsSelector->setEnabled( mChkCustomCRS->isChecked() );
  updateParametersFromReferenceLayer();
}

void QgsAlignRasterDialog::updateCustomCellSize()
{
  mSpinCellSizeX->setEnabled( mChkCustomCellSize->isChecked() );
  mSpinCellSizeY->setEnabled( mChkCustomCellSize->isChecked() );
  updateParametersFromReferenceLayer();
}

void QgsAlignRasterDialog::updateCustomGridOffset()
{
  mSpinGridOffsetX->setEnabled( mChkCustomGridOffset->isChecked() );
  mSpinGridOffsetY->setEnabled( mChkCustomGridOffset->isChecked() );
  updateParametersFromReferenceLayer();
}


void QgsAlignRasterDialog::runAlign()
{
  setEnabled( false );

  const bool res = mAlign->run();

  setEnabled( true );

  if ( res )
  {
    if ( mChkAddToCanvas->isChecked() )
    {
      const auto constRasters = mAlign->rasters();
      for ( const QgsAlignRaster::Item &item : constRasters )
      {
        QgsRasterLayer *layer = new QgsRasterLayer( item.outputFilename, QFileInfo( item.outputFilename ).baseName() );
        if ( layer->isValid() )
          QgsProject::instance()->addMapLayer( layer );
        else
          delete layer;
      }
    }
  }
  else
  {
    QMessageBox::critical( this, tr( "Align Rasters" ), tr( "Failed to align rasters:" ) + "\n\n" + mAlign->errorMessage() );
  }
}


// ------


QgsAlignRasterLayerConfigDialog::QgsAlignRasterLayerConfigDialog()
{
  setWindowTitle( tr( "Configure Layer Resampling" ) );
  QVBoxLayout *layout = new QVBoxLayout();

  cboLayers = new QgsMapLayerComboBox( this );
  cboLayers->setFilters( QgsMapLayerProxyModel::RasterLayer );

  cboResample = new QComboBox( this );
  cboResample->addItem( tr( "Nearest Neighbour" ), QgsAlignRaster::RA_NearestNeighbour );
  cboResample->addItem( tr( "Bilinear (2x2 Kernel)" ), QgsAlignRaster::RA_Bilinear );
  cboResample->addItem( tr( "Cubic (4x4 Kernel)" ), QgsAlignRaster::RA_Cubic );
  cboResample->addItem( tr( "Cubic B-Spline (4x4 Kernel)" ), QgsAlignRaster::RA_CubicSpline );
  cboResample->addItem( tr( "Lanczos (6x6 Kernel)" ), QgsAlignRaster::RA_Lanczos );
  cboResample->addItem( tr( "Average" ), QgsAlignRaster::RA_Average );
  cboResample->addItem( tr( "Mode" ), QgsAlignRaster::RA_Mode );
  cboResample->addItem( tr( "Maximum" ), QgsAlignRaster::RA_Max );
  cboResample->addItem( tr( "Minimum" ), QgsAlignRaster::RA_Min );
  cboResample->addItem( tr( "Median" ), QgsAlignRaster::RA_Median );
  cboResample->addItem( tr( "First Quartile (Q1)" ), QgsAlignRaster::RA_Q1 );
  cboResample->addItem( tr( "Third Quartile (Q3)" ), QgsAlignRaster::RA_Q3 );

  editOutput = new QLineEdit( this );
  btnBrowse = new QPushButton( tr( "Browse…" ), this );
  connect( btnBrowse, &QAbstractButton::clicked, this, &QgsAlignRasterLayerConfigDialog::browseOutputFilename );

  QHBoxLayout *layoutOutput = new QHBoxLayout();
  layoutOutput->addWidget( editOutput );
  layoutOutput->addWidget( btnBrowse );

  chkRescale = new QCheckBox( tr( "Rescale values according to the cell size" ), this );

  btnBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  connect( btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept );
  connect( btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject );

  layout->addWidget( new QLabel( tr( "Input raster layer:" ), this ) );
  layout->addWidget( cboLayers );
  layout->addWidget( new QLabel( tr( "Output raster filename:" ), this ) );
  layout->addLayout( layoutOutput );
  layout->addWidget( new QLabel( tr( "Resampling method:" ), this ) );
  layout->addWidget( cboResample );
  layout->addWidget( chkRescale );
  layout->addWidget( btnBox );
  setLayout( layout );
}

QString QgsAlignRasterLayerConfigDialog::inputFilename() const
{
  QgsRasterLayer *l = qobject_cast<QgsRasterLayer *>( cboLayers->currentLayer() );
  return l ? l->source() : QString();
}

QString QgsAlignRasterLayerConfigDialog::outputFilename() const
{
  return editOutput->text();
}

QgsAlignRaster::ResampleAlg QgsAlignRasterLayerConfigDialog::resampleMethod() const
{
  return static_cast< QgsAlignRaster::ResampleAlg >( cboResample->currentData().toInt() );
}

bool QgsAlignRasterLayerConfigDialog::rescaleValues() const
{
  return chkRescale->isChecked();
}

void QgsAlignRasterLayerConfigDialog::setItem( const QString &inputFilename, const QString &outputFilename,
    QgsAlignRaster::ResampleAlg resampleMethod, bool rescaleValues )
{
  cboLayers->setLayer( _rasterLayer( inputFilename ) );
  editOutput->setText( outputFilename );
  cboResample->setCurrentIndex( cboResample->findData( resampleMethod ) );
  chkRescale->setChecked( rescaleValues );
}

void QgsAlignRasterLayerConfigDialog::browseOutputFilename()
{
  const QgsSettings settings;
  const QString dirName = editOutput->text().isEmpty() ? settings.value( QStringLiteral( "UI/lastRasterFileDir" ), QDir::homePath() ).toString() : editOutput->text();

  QString fileName = QFileDialog::getSaveFileName( this, tr( "Select output file" ), dirName, tr( "GeoTIFF" ) + " (*.tif *.tiff *.TIF *.TIFF)" );

  if ( !fileName.isEmpty() )
  {
    // ensure the user never omitted the extension from the file name
    if ( !fileName.endsWith( QLatin1String( ".tif" ), Qt::CaseInsensitive ) && !fileName.endsWith( QLatin1String( ".tiff" ), Qt::CaseInsensitive ) )
    {
      fileName += QLatin1String( ".tif" );
    }
    editOutput->setText( fileName );
  }
}
