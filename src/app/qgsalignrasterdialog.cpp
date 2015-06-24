#include "qgsalignrasterdialog.h"

#include "qgsapplication.h"
#include "qgsalignraster.h"
#include "qgsdataitem.h"
#include "qgsmaplayercombobox.h"
#include "qgsmaplayerregistry.h"
#include "qgsrasterlayer.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QVBoxLayout>


static QgsMapLayer* _rasterLayer( const QString& filename )
{
  QMap<QString, QgsMapLayer*> layers = QgsMapLayerRegistry::instance()->mapLayers();
  foreach ( QgsMapLayer* layer, layers.values() )
  {
    if ( layer->type() == QgsMapLayer::RasterLayer && layer->source() == filename )
      return layer;
  }
  return 0;
}

static QString _rasterLayerName( const QString& filename )
{
  if ( QgsMapLayer* layer = _rasterLayer( filename ) )
    return layer->name();

  QFileInfo fi( filename );
  return fi.baseName();
}



/** Helper class to report progress */
struct QgsAlignRasterDialogProgress : public QgsAlignRaster::ProgressHandler
{
  QgsAlignRasterDialogProgress( QProgressBar* pb ) : mPb( pb ) {}
  virtual bool progress( double complete )
  {
    mPb->setValue(( int ) qRound( complete * 100 ) );
    qApp->processEvents(); // to actually show the progress in GUI
    return true;
  }

protected:
  QProgressBar* mPb;
};


QgsAlignRasterDialog::QgsAlignRasterDialog( QWidget *parent )
    : QDialog( parent )
{
  setupUi( this );

  mBtnAdd->setIcon( QIcon( QgsApplication::iconPath( "symbologyAdd.svg" ) ) );
  mBtnEdit->setIcon( QIcon( QgsApplication::iconPath( "symbologyEdit.png" ) ) );
  mBtnRemove->setIcon( QIcon( QgsApplication::iconPath( "symbologyRemove.svg" ) ) );

  mAlign = new QgsAlignRaster;
  mAlign->setProgressHandler( new QgsAlignRasterDialogProgress( mProgress ) );

  connect( mBtnAdd, SIGNAL( clicked( bool ) ), this, SLOT( addLayer() ) );
  connect( mBtnRemove, SIGNAL( clicked( bool ) ), this, SLOT( removeLayer() ) );
  connect( mBtnEdit, SIGNAL( clicked( bool ) ), this, SLOT( editLayer() ) );

  connect( mCboReferenceLayer, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateConfigFromReferenceLayer() ) );

  mClipExtentGroupBox->setChecked( false );

  // TODO: auto-detect reference layer

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( runAlign() ) );

  populateLayersView();
}

QgsAlignRasterDialog::~QgsAlignRasterDialog()
{
  delete mAlign;
}


void QgsAlignRasterDialog::populateLayersView()
{
  mCboReferenceLayer->clear();

  QStandardItemModel* model = new QStandardItemModel();
  foreach ( QgsAlignRaster::Item item, mAlign->rasters() )
  {
    QString layerName = _rasterLayerName( item.inputFilename );

    QStandardItem* si = new QStandardItem( QgsLayerItem::iconRaster(), layerName );
    model->appendRow( si );

    mCboReferenceLayer->addItem( layerName );
  }

  mViewLayers->setModel( model );

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( model->rowCount() > 0 );
}

void QgsAlignRasterDialog::addLayer()
{
  QgsAlignRasterLayerConfigDialog d;
  if ( !d.exec() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();

  QgsAlignRaster::Item item( d.inputFilename(), d.outputFilename() );
  item.resampleMethod = ( QgsAlignRaster::ResampleAlg ) d.resampleMethod();
  item.rescaleValues = d.rescaleValues();
  list.append( item );

  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::removeLayer()
{
  QModelIndex current = mViewLayers->currentIndex();
  if ( !current.isValid() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();
  list.removeAt( current.row() );
  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::editLayer()
{
  QModelIndex current = mViewLayers->currentIndex();
  if ( !current.isValid() )
    return;

  QgsAlignRaster::List list = mAlign->rasters();
  QgsAlignRaster::Item item = list.at( current.row() );

  QgsAlignRasterLayerConfigDialog d;
  d.setItem( item.inputFilename, item.outputFilename, item.resampleMethod, item.rescaleValues );
  if ( !d.exec() )
    return;

  QgsAlignRaster::Item itemNew( d.inputFilename(), d.outputFilename() );
  itemNew.resampleMethod = ( QgsAlignRaster::ResampleAlg ) d.resampleMethod();
  itemNew.rescaleValues = d.rescaleValues();
  list[current.row()] = itemNew;
  mAlign->setRasters( list );

  populateLayersView();
}

void QgsAlignRasterDialog::updateConfigFromReferenceLayer()
{
  int index = mCboReferenceLayer->currentIndex();
  if ( index < 0 )
    return;

  mAlign->setParametersFromRaster( mAlign->rasters().at( index ).inputFilename );

  QgsCoordinateReferenceSystem destCRS( mAlign->destinationCRS() );
  mCrsSelector->setCrs( destCRS );

  QSizeF cellSize = mAlign->cellSize();
  mSpinCellSizeX->setValue( cellSize.width() );
  mSpinCellSizeY->setValue( cellSize.height() );

  QPointF gridOffset = mAlign->gridOffset();
  mSpinGridOffsetX->setValue( gridOffset.x() );
  mSpinGridOffsetY->setValue( gridOffset.y() );

  mClipExtentGroupBox->setOriginalExtent( mAlign->clipExtent(), destCRS );
}

void QgsAlignRasterDialog::runAlign()
{
  setEnabled( false );

  bool res = mAlign->run();

  setEnabled( true );

  if ( res )
  {
    if ( mChkAddToCanvas->isChecked() )
    {
      foreach ( const QgsAlignRaster::Item& item, mAlign->rasters() )
      {
        QgsRasterLayer* layer = new QgsRasterLayer( item.outputFilename, QFileInfo( item.outputFilename ).baseName() );
        if ( layer->isValid() )
          QgsMapLayerRegistry::instance()->addMapLayer( layer );
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
  QVBoxLayout* layout = new QVBoxLayout();

  cboLayers = new QgsMapLayerComboBox( this );
  cboLayers->setFilters( QgsMapLayerProxyModel::RasterLayer );

  cboResample = new QComboBox( this );
  QStringList methods;
  methods << tr( "Nearest neighbour" ) << tr( "Bilinear (2x2 kernel)" )
  << tr( "Cubic (4x4 kernel)" ) << tr( "Cubic B-Spline (4x4 kernel)" ) << tr( "Lanczos (6x6 kernel)" )
  << tr( "Average" ) << tr( "Mode" );
  cboResample->addItems( methods );

  editOutput = new QLineEdit( this );
  btnBrowse = new QPushButton( tr( "Browse..." ), this );
  connect( btnBrowse, SIGNAL( clicked( bool ) ), this, SLOT( browseOutputFilename() ) );

  QHBoxLayout* layoutOutput = new QHBoxLayout();
  layoutOutput->addWidget( editOutput );
  layoutOutput->addWidget( btnBrowse );

  chkRescale = new QCheckBox( tr( "Rescale values according to the cell size" ), this );

  btnBox = new QDialogButtonBox( QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this );
  connect( btnBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( btnBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

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
  QgsRasterLayer* l = qobject_cast<QgsRasterLayer*>( cboLayers->currentLayer() );
  return l ? l->source() : QString();
}

QString QgsAlignRasterLayerConfigDialog::outputFilename() const
{
  return editOutput->text();
}

int QgsAlignRasterLayerConfigDialog::resampleMethod() const
{
  return cboResample->currentIndex();
}

bool QgsAlignRasterLayerConfigDialog::rescaleValues() const
{
  return chkRescale->isChecked();
}

void QgsAlignRasterLayerConfigDialog::setItem( const QString& inputFilename, const QString& outputFilename,
    int resampleMethod, bool rescaleValues )
{
  cboLayers->setLayer( _rasterLayer( inputFilename ) );
  editOutput->setText( outputFilename );
  cboResample->setCurrentIndex( resampleMethod );
  chkRescale->setChecked( rescaleValues );
}

void QgsAlignRasterLayerConfigDialog::browseOutputFilename()
{
  QSettings settings;
  QString dirName = editOutput->text().isEmpty() ? settings.value( "/UI/lastRasterFileDir", "." ).toString() : editOutput->text();

  QString fileName = QFileDialog::getSaveFileName( this, tr( "Select output file" ), dirName, tr( "GeoTIFF" ) + " (*.tif *.tiff *.TIF *.TIFF)" );

  if ( !fileName.isEmpty() )
  {
    // ensure the user never ommited the extension from the file name
    if ( !fileName.toLower().endsWith( ".tif" ) && !fileName.toLower().endsWith( ".tiff" ) )
    {
      fileName += ".tif";
    }
    editOutput->setText( fileName );
  }
}
